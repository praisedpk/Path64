/*
 * Copyright (C) 2007. PathScale, LLC. All Rights Reserved.
 */
/*
 * Copyright 2003, 2004, 2005, 2006 PathScale, Inc.  All Rights Reserved.
 */

/*

  Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.

   Path64 is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   Path64 is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with Path64; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

   Special thanks goes to SGI for their continued support to open source

*/


#if defined(BUILD_OS_DARWIN)
#include <darwin_elf.h>
#else /* defined(BUILD_OS_DARWIN) */
#include <elf.h>
#endif /* defined(BUILD_OS_DARWIN) */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#include <stdio.h>
#include <string.h>
#include <vector>
#include <algorithm>
#include <stack>
#include <map>
#include <tree.h>
#include "obj_info.h"
#include "lib_phase_dir.h"

#define PAGESIZE 65536
int debug = 0;
const int LEN = 1000;  // max line size
const int MIN_FREQ = 15; 


// strcmp function object
struct ltstr
{
  bool operator()(const char* s1, const char* s2) const {
    return strcmp(s1, s2) < 0;
  }
};


// procedure 
struct PROC {
  char *name;
  int  size;         // size of myself
  int  size_cluster; // size of my cluster
  PROC *base;        // layout
  int  offset;
  bool emit;  
  int  src_order;
  PROC(char *n) {
    name = strdup(n); size = 0; size_cluster = 0; 
    base = NULL; offset = 0; emit = false; src_order = 0;
  }
};


// ordering for procedure layout
struct less<PROC> {
  bool operator()(const PROC *p1, const PROC *p2) {
    char *name1 = p1->base == NULL ? p1->name : p1->base->name;
    char *name2 = p2->base == NULL ? p2->name : p2->base->name;
    if (p1->base != p2->base) {
      int cmp = strcmp(name1, name2);
      if (cmp < 0)
	return true;
      else if (cmp >= 0)
	return false;
    }
    return p1->offset < p2->offset;
  }
};


// ordering for source order layout
template <class PROC>
struct less_src_order {
  bool operator()(const PROC *p1, const PROC *p2) {
    return (p1->src_order < p2->src_order);
  }
};

// an edge of the call-graph
struct CALL {
  PROC *caller;
  PROC *callee;
  int  freq;
  int  retry_count; 
  CALL(PROC *f1, PROC *f2, int f):caller(f1),callee(f2),freq(f),retry_count(0) {};
};


// order the call-graph edge with frequency
struct less<CALL> {
  bool operator()(const CALL &c1, const CALL &c2) {
    return (c1.freq < c2.freq);
  }
};


typedef pair<PROC *, PROC *> map_key;
typedef map< map_key, int> CG;
typedef map<const char *, PROC *, ltstr> PROCTAB;

// Returns true if path refers to an ordinary file.
bool file_exists(const char* path)
{
  if (!path || strlen(path) == 0)
    return false;
 
  struct stat buf;
  return stat(path, &buf) == 0 && S_ISREG(buf.st_mode);
}                                                             
// compress the relative layout into absolute layout
// to the base.
struct compress_path : public unary_function<PROC *, void>
{
  void operator() (PROC *x) { 
    PROC *base = x;  
    int  offset = x->offset;
    while (base->base != NULL) {
      base = base->base;
      offset += base->offset;
    }
    if (x != base) {
      x->base = base;
      x->offset = offset;
    }
  }
};


// read in the dynamic call graph generated by IPA
//
void read_call_graph(const char *cgraph,
		     PROCTAB &procedures,
		     CG &cg)
{
  FILE *in1 = fopen(cgraph, "r");
  if (in1 == NULL) {
    perror(cgraph);
    exit(1);
  }
  
  char buffer[LEN];
  char *callee, *caller;
  
  while (fgets(buffer, LEN, in1)) {
    caller = strdup(strtok(buffer, " \t"));
    callee = strdup(strtok(NULL, " \t"));
    int freq = atoi(strtok(NULL, " \t"));
    if (debug)
      printf("input call graph: %s %s %d\n", caller, callee, freq);
    
    PROC *p1 = new PROC(caller);
    PROC *p2 = new PROC(callee);
    procedures.insert(pair<const char *, PROC *>(p1->name, p1));
    procedures.insert(pair<const char *, PROC *>(p2->name, p2));

    map_key key(procedures[caller], procedures[callee]);
    CG::iterator it = cg.find(key);
    if (it != cg.end()) {
      (*it).second += freq;
    } else 
      cg.insert(pair<map_key,int>(key, freq));
  }
  fclose(in1);
}


// keep track of the order if cord is not run.
int src_order = 0;  

// read in size of procedures from the .o files
//
template <class Shdr>
void process_object_file (File_Info &fi, Shdr *tag, PROCTAB &procedures)
{
  Proc_Iter<Shdr> it(fi.begin(tag));

  for (it = fi.begin(tag); it != fi.end(tag); ++it) {
    char *p = (*it).first;
    int  size = (*it).second;
    bool updated = false;
    if (strncmp(p, ".text.", 6) == 0) {
      char *procname = p+6;
      PROCTAB::iterator ip = procedures.find(procname);
      if (ip != procedures.end()) {
	if (size % 16 != 0)
	  size = size + (16 - size % 16);
	if (debug)
	  printf("update %s size to %d\n",  (*ip).second->name, size);
	(*ip).second->size = size;
	(*ip).second->size_cluster = size;
	updated = true;
	(*ip).second->src_order = src_order++;
      }
    }
    if (!updated && strcmp(p,".text.") != 0)
      if (debug)
	printf("cannot find %s in call-graph\n", p);
  }
}


// read all objects listed in the objlist file.
void read_obj_list(const char *objlist, PROCTAB &procedures)
{
  FILE *in = fopen(objlist, "r");
  if (in == NULL) {
    perror(objlist);
    exit(1);
  }
  
  char buffer[LEN];
  while (fgets(buffer, LEN, in)) {
    char *obj_file = strtok(buffer, " \t\n");
    if (debug) 
      printf("process object %s\n", obj_file);
    File_Info fi (obj_file);
    if (fi.Get_obj_class () == ELFCLASS32)
      process_object_file (fi, Shdr32_tag (), procedures);
    else
      process_object_file (fi, Shdr64_tag (), procedures);
  }
  fclose(in);
}


//  return true if layout is successful or too late
//  to change.
bool layout(PROC *p1, PROC *p2, int freq, int retry_count)
{
  if (p1 == p2) return true;

  if (debug) {
    if (p1->size == 0) 
      printf("warning: %s has 0 size.\n", p1->name);
    if (p2->size == 0)
      printf("warning: %s has 0 size.\n", p2->name);
  }

  compress_path()(p1);
  compress_path()(p2);

  PROC *base1 = p1->base ? p1->base : p1;
  PROC *base2 = p2->base ? p2->base : p2;

  int loc1 = p1->offset % PAGESIZE;
  int loc2 = (p2->offset + base1->size_cluster) % PAGESIZE;
  if (base1 == base2)
    loc2 = p2->offset % PAGESIZE;

  if (loc2 < loc1) loc2 += PAGESIZE;
  int overlap;
  if (loc2 >= loc1+p1->size)
    overlap = 0;
  else
    overlap = loc1 + p1->size - loc2;
  if (overlap == 0) {
    if (loc1 <= loc2) loc1 += PAGESIZE;
    if (loc1 >= loc2 + p2->size)
      overlap = 0;
    else
      overlap = loc2 + p2->size - loc1;
  }
 
  if (overlap > 0) {
    if (debug) {
      printf("layout: detected overlap %d between %s (%d) <%s,%d> and %s (%d) <%s,%d> with freq %d\n",
	     overlap,
	     p1->name, p1->size, base1->name, p1->offset,
	     p2->name, p2->size, base2->name, p2->offset,
	     freq);
    }
    if (base1 != base2) {
      if (debug)
	printf("layout: delay layout\n");
      return false;
    } else {
      if (debug)
	printf("too late to change layout\n");
      return true;
    }
  }

  if (base1 != base2) {
    if (debug)
      printf("layout: %s <%s,%d,%d> at %s <%s,%d,%d> with offset %d  freq=%d retry=%d\n", 
	     p2->name, base2->name, p2->offset, p2->size,
	     p1->name, base1->name, p1->offset, p1->size,
	     base1->size_cluster, freq, retry_count);
  
    base2->base = base1;
    base2->offset = base1->size_cluster;
    base1->size_cluster += base2->size_cluster;
    base2->size_cluster = 0;
  }

  p1->emit = true;  // layout changed
  p2->emit = true;
  return true;
}


// emit lkcord file readable by ld -orderspec
//
void emit_lkcord(const char *outfile, vector<PROC*> &proc_layout, vector<PROC*> &src_order_layout)
{
  FILE *out;
  FILE *previous;
  FILE *bottom;
  char buffer[LEN];
  char* toolroot = getenv("TOOLROOT"); 
  char* previous_script= BINPATH "/" "cord_prologue";
  char* bottom_script=  BINPATH "/cord_epilogue";
  char* previous_file;
  char* bottom_file;
  int len;


  if (toolroot) {
	len = strlen(toolroot) + strlen(previous_script);
	previous_file = (char *)malloc(len + 1);
	len = strlen(toolroot) + strlen(bottom_script);
	bottom_file = (char *)malloc(len + 1);
	sprintf(previous_file,"%s%s", toolroot, previous_script); 
	sprintf(bottom_file,"%s%s", toolroot, bottom_script); 
  } else {
	previous_file = previous_script;
	bottom_file = bottom_script;
  }
  if (outfile != NULL) {
    out = fopen(outfile, "w");
    if (out == NULL) {
      perror(outfile);
      exit(1);
    }
  } else 
    out = stdout;

  previous = fopen(previous_file, "r");
  if (previous == NULL ) {
      printf("Can't open cord-prologue script file\n");
      perror("previous");
      exit(1);
  }

  bottom = fopen(bottom_file, "r");
  if (bottom == NULL ) {
      printf("Can't open cord-epilogue script file\n");
      perror("bottom");
      exit(1);
  }

  while (fgets(buffer, LEN, previous)) 
	fputs(buffer, out);

  vector<PROC *>::iterator q;
  for (q = proc_layout.begin(); q != proc_layout.end(); q++) {
    PROC *proc = *q;
    if (debug)
      printf("sorted:  %s at %s + %d\n", 
	     proc->name,
	     proc->base ? proc->base->name : "null" , 
	     proc->offset);
    fprintf(out, "    *(.text.%s)\n", proc->name);
  }

  for (q = src_order_layout.begin(); q != src_order_layout.end(); q++) {
    PROC *proc = *q;
    if (debug)
      printf("src_order:  %s\n", proc->name);
    fprintf(out, "    *(.text.%s)\n", proc->name);
  }

  while (fgets(buffer, LEN, bottom)) 
	fputs(buffer, out);

  fclose(previous);
  fclose(bottom);
}


main(int argc, char *argv[])
{
  char *outfile = NULL;

  if (argc < 3) {
    fprintf(stderr, "usage: gen_cord -o orderspec call-graph objfilelist\n");
    exit(1);
  }
  
  while (argv[1][0] == '-') {
    char *option = &argv[1][1];
    if (strcmp(option, "d") == 0)
      debug = 1;
    if (strcmp(option, "o") == 0) {
      ++argv;
      --argc;
      outfile = argv[1];
    }
    ++argv;
    --argc;
  } 
  char *cgraph = argv[1];
  char *objfilelist = argv[2];

  CG cg;
  PROCTAB procedures;
  priority_queue<CALL> Q;

  read_call_graph(cgraph, procedures, cg);

  // form a priority queue sorted in increasing order of freq
  CG::iterator it;
  for (it = cg.begin(); it != cg.end(); it++) {
    PROC *p1 = (*it).first.first;
    PROC *p2 = (*it).first.second;
    int freq = (*it).second;
    if (debug)
      printf("summed call graph: %s %s %d\n", p1->name, p2->name, freq);
    Q.push(*(new CALL(p1, p2, freq)));
  }

  // process .o files to collect procedures sizes
  read_obj_list(objfilelist, procedures);

  // start procedure layout 
  while (!Q.empty()) {
    PROC *callee = Q.top().caller;
    PROC *caller = Q.top().callee;
    int freq = Q.top().freq;
    int retry = Q.top().retry_count;

    // ignore call edge with low frequency
    if (freq <= MIN_FREQ) break;

    if (!layout(caller, callee, freq, retry)) {
      CALL edge = Q.top();
      edge.freq /= 2;  // divide by 2
      edge.retry_count++;
      Q.pop();
      Q.push(edge);
   } else
      Q.pop();
  }

  // generate lkcord order vector by sorting the offsets relative to
  // the base procedure
  vector<PROC *> proc_layout, src_order_layout;
  PROCTAB::iterator p;
  for (p = procedures.begin(); p != procedures.end(); p++) {
    PROC *proc = (*p).second;
    if (proc->emit) {
      compress_path()(proc);
      proc_layout.push_back(proc);
    } else {
      src_order_layout.push_back(proc);
    }
  }
  sort(proc_layout.begin(), proc_layout.end(),  less<PROC>());
  sort(src_order_layout.begin(), src_order_layout.end(),  less_src_order<PROC>());

  // emit lkcord file readable by ld -orderspec 
  emit_lkcord(outfile, proc_layout, src_order_layout);
}


