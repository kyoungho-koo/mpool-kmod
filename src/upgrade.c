// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2015-2020 Micron Technology, Inc.  All rights reserved.
 */

/* DOC: Module info.
 *
 * Pool metadata upgrade module.
 *
 * Defines functions used to upgrade the mpool metadata.
 *
 */

#include "mpcore_defs.h"

/*
 * Latest mpool MDC content version understood by this binary.
 * Also version used to write MDC content by this binary.
 */
#define MDCVER_MAJOR       1
#define MDCVER_MINOR       0
#define MDCVER_PATCH       0
#define MDCVER_DEV         0

/**
 * struct mdcver_info - mpool MDC content version and its information.
 *
 * Such a structure instance is added each time the mpool MDCs content
 * semantic/format changes (making it incompatible with earlier binary
 * versions).
 *
 * @mi_mdcver:  version of a mpool MDC content. It is the version of the
 *              first binary that introduced that content semantic/format.
 * @mi_types:   types used by this release (when writing MDC0-N content)
 * @mi_ntypes:  no. of types are used by this release.
 * @mi_comment: comment about that version
 */
struct mdcver_info {
	struct omf_mdcver   mi_mdcver;
	uint8_t            *mi_types;
	uint8_t             mi_ntypes;
	const char         *mi_comment;
};

/*
 * mpool MDC types used when MDC content is written at version 1.0.0.0.
 */
uint8_t mdcver_1_0_0_0_types[] = {
	OMF_MDR_OCREATE, OMF_MDR_OUPDATE, OMF_MDR_ODELETE, OMF_MDR_OIDCKPT,
	OMF_MDR_OERASE, OMF_MDR_MCCONFIG, OMF_MDR_MCSPARE, OMF_MDR_VERSION,
	OMF_MDR_MPCONFIG};


/*
 * mdcver_info mdcvtab[] - table of versions of mpool MDCs content.
 *
 * Each time MDC content semantic/format changes (making it incompatible
 * with earlier binary versions) an entry is added in this table.
 * The entry at the end of the array (highest index) is the version placed
 * in the mpool MDC version record written to media when this binary writes
 * the mpool MDCs.
 * This entry is also the last mpool MDC content format/semantic that this
 * binary understands.
 *
 * Example:
 * - Initial binary 1.0.0.0 generates first ever MDCs content.
 *   There is one entry in the table with its mi_mdcver being 1.0.0.0.
 * - binary 1.0.0.1 is released and changes mpool MDC content semantic (for
 *   example chenge the meaning of media class enum). This release adds the
 *   entry 1.0.0.1 in this table.
 * - binary 1.0.1.0 is released and doesn't change MDCs content semantic/format,
 *   MDCs content generated by 1.0.1.0 binary is still compatible with a
 *   1.0.0.1 binary reading it.
 *   No entry is added in the table.
 * - binary 2.0.0.0 is released and it changes MDCs content semantic.
 *   A third entry is added in the table with its mi_mdcver being 2.0.0.0.
 */
struct mdcver_info mdcvtab[] = {
	{{ {MDCVER_MAJOR, MDCVER_MINOR, MDCVER_PATCH, MDCVER_DEV} },
	mdcver_1_0_0_0_types, sizeof(mdcver_1_0_0_0_types),
	"Initial mpool MDCs content"},
};

struct omf_mdcver *omfu_mdcver_cur(void)
{
	return &mdcvtab[ARRAY_SIZE(mdcvtab) - 1].mi_mdcver;
}

const char *omfu_mdcver_comment(struct omf_mdcver *mdcver)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(mdcvtab); i++)
		if (omfu_mdcver_cmp(mdcver, "==", &mdcvtab[i].mi_mdcver))
			return mdcvtab[i].mi_comment;

	return NULL;
}

char *omfu_mdcver_to_str(struct omf_mdcver *mdcver, char *buf, size_t sz)
{
	snprintf(buf, sz, "%u.%u.%u.%u", mdcver->mdcv_major,
		 mdcver->mdcv_minor, mdcver->mdcv_patch, mdcver->mdcv_dev);

	return buf;
}

bool omfu_mdcver_cmp(struct omf_mdcver *a, char *op, struct omf_mdcver *b)
{
	size_t cnt = ARRAY_SIZE(a->mdcver);
	int    i;
	int    res = 0;

	for (i = 0; i < cnt; i++) {
		if (a->mdcver[i] != b->mdcver[i]) {
			res = (a->mdcver[i] > b->mdcver[i]) ? 1 : -1;
			break;
		}
	}

	if (((op[1] == '=') && (res == 0)) || ((op[0] == '>') && (res > 0)) ||
	    ((op[0] == '<') && (res < 0)))
		return true;

	return false;
}

bool omfu_mdcver_cmp2(struct omf_mdcver *a, char *op, u16 major, u16 minor, u16 patch, u16 dev)
{
	struct omf_mdcver  b;

	b.mdcv_major = major;
	b.mdcv_minor = minor;
	b.mdcv_patch = patch;
	b.mdcv_dev   = dev;

	return omfu_mdcver_cmp(a, op, &b);
}
