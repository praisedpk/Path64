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


#ifndef __IR_BWRITE_H__
#define __IR_BWRITE_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

				       
/*
 * This flag is usually FALSE, but bedriver may set it to TRUE so that
 * the backend's internal maps will be written out.
 */
#ifdef BACK_END
extern BOOL Write_BE_Maps;
extern BOOL Write_AC_INTERNAL_Map;
extern BOOL Write_ALIAS_CLASS_Map;
#endif /* BACK_END */


/*
 * Exported interface for writing WHIRL in binary form to a file.
 * All functions return -1 upon failure, which should be treated as
 * a fatal error and the process should be aborted.  Also, "errno" will
 * be set and can be used for printing appropriate error messages.
 * If the operation is successful, 0 will be returned.
 */


typedef struct section {
    const char *name;			    /* section name */
    Elf64_Shdr shdr;		    /* Elf section */
} Section;


typedef struct output_file {
    char *file_name;
    INT output_fd;		    /* file id for output file */
    char *map_addr; 		    /* base address of the mapped region */
    off_t mapped_size;		    /* max. size of the mapped region */
    off_t file_size;
    Section *section_list;	    /* array of Elf sections */
    INT max_num_of_section;
    INT num_of_section;
    Section *cur_section;	    /* set by get_section() */
} Output_File;


extern Output_File *Current_Output;

/* Call these once at the beginning and end, respectively. */
extern Output_File *WN_open_output (char *file_name);
extern void WN_close_output (Output_File *fl);
extern void WN_close_file (void *fl);


/*
 * Write global tables to their own Elf sections.  These should only be
 * called after all the PUs have been written out.
 */

extern void WN_write_globals (Output_File *fl);
extern void WN_write_dst (void *dst, Output_File *fl);
extern void WN_write_strtab (const void *strtab, UINT64 size, Output_File *fl);
extern void WN_write_localmap (void *localmap, Output_File *fl);
extern void IPA_write_summary (void (*IPA_irb_write_summary) (Output_File*),
			      Output_File *fl);
extern void IPA_copy_PU(PU_Info *pu, char *section_base, Output_File *outfile);

extern void WN_write_flags (INT argc, char **argv, Output_File *fl);
extern void WN_write_revision (Output_File *fl);
extern void WN_close_file (void *this_fl);



/*
 * Write PU section header.  This must be called after writing out all the
 * PUs but before writing any of the global tables.
 */

extern void WN_write_PU_Infos (PU_Info *pu_list, Output_File *fl);


/*
 * Write PU-specific structions to subsections of the PU section.
 * WN_write_tree uses the "off_map" mapping to store the subsection offsets
 * of the WHIRL nodes that may be referenced by the dependence graph or
 * prefetch pointers.
 */

extern void WN_write_tree (PU_Info *pu, WN_MAP off_map, Output_File *fl);
extern void WN_write_symtab (PU_Info *pu, Output_File *fl);
extern void WN_write_depgraph (PU_Info *pu, WN_MAP off_map, Output_File *fl);
extern void WN_write_prefetch (PU_Info *pu, WN_MAP off_map, Output_File *fl);
extern void WN_write_feedback (PU_Info *pu, Output_File *fl);
extern void WN_write_INT32_map(PU_Info     *pu,
			       WN_MAP       off_map,
			       Output_File *fl,
			       INT32        subsection_type,
			       WN_MAP       value_map,
			       const char  *subsection_name);
extern void WN_write_voidptr_map(PU_Info     *pu,
				 WN_MAP       off_map,
				 Output_File *fl,
				 INT32        subsection_type,
				 WN_MAP       value_map,
				 const char  *subsection_name);
#ifndef OWN_ERROR_PACKAGE

/*
 * Define common routines for writing all the whirl sections.
 * These routines use the standard compiler error reporting mechanism.
 */

extern Output_File *Open_Output_Info (char *output_file);
extern void Write_PU_Info (PU_Info *pu);
extern void Write_Global_Info (PU_Info *pu_tree);
extern void Close_Output_Info (void);
extern void WN_write_elf_symtab (const void* symtab, UINT64 size,
				 UINT64 entsize, UINT align, Output_File* fl);

#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */
    
#endif /* __IR_BWRITE_H__ */
