/*
 * Copyright (C) 2015 Josh Poimboeuf <jpoimboe@redhat.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _SPECIAL_H
#define _SPECIAL_H

#include <stdbool.h>
#include "objtool.h"
#include "elf.h"

struct instruction;

struct special_alt {
	struct list_head list;

	bool group;
	bool skip_orig;
	bool jump_or_nop;

	struct section *orig_sec;
	unsigned long orig_off;

	struct section *new_sec;
	unsigned long new_off;

	unsigned int orig_len, new_len; /* group only */
};

struct alternative {
	struct list_head list;
	struct instruction *insn;
};

extern const char * const arch_noreturns[];
extern int arch_noreturns_size;

int special_get_alts(struct elf *elf, struct list_head *alts);

void arch_handle_alternative(unsigned short feature, struct special_alt *alt);

void arch_mark_func_jump_tables(struct objtool_file *file,
				    struct symbol *func);

int arch_dynamic_add_jump_table_alts(struct list_head *p_orbit_list, struct objtool_file *file,
			struct symbol *func, struct instruction *insn);

#endif /* _SPECIAL_H */
