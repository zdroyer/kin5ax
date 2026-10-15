#ifndef KIN5AXFCM_H
#define KIN5AXFCM_H

/* kin5axfcm.h - the FreeCAD machine file (.fcm) bridge of kin5ax, in C.
 *
 * A second header over the same shared library as kin5ax.h, which it includes: the handles, the
 * statuses and k5_last_error are those of that header, and a chain opened here is a k5_chain like
 * any other. It stands apart because the conversion is a different thing from the kinematics - it is
 * lossy, it takes conventions that are not in the file, and it answers with remarks - and a host
 * that never touches FreeCAD machine files should not have to read about it.
 *
 * WHAT THE TWO FILES ARE. A .fcm names every axis and gives its joint as an origin and a direction
 * in the base frame of the machine with all axes at zero; "role" puts the axis in the head or the
 * table branch, "parent" and "sequence" order them. A kinematic chain of kin5ax gives every segment
 * a constant transformation relative to the one before it, moves along or about the local OZ, and
 * closes each branch with a terminal. Neither file holds everything the other one has.
 *
 * SO NOTHING IS DROPPED SILENTLY. Every conversion fills a report of remarks - a base frame that was
 * not the identity, an axis with no role, tool heads that a kinematic chain has no place for, the
 * collision boxes an .fcm cannot carry - and k5_fcm_remark_count with k5_fcm_remark read them back
 * off the chain. A host that ignores them is hiding from its user the difference between the machine
 * in the file and the machine it is about to solve for.
 *
 * TWO THINGS WORTH KNOWING BEFORE USING ONE. An .fcm has no collision boxes at all, so a machine
 * imported from one is a stick figure: a collision check against it will find nothing, whatever the
 * tool does, and that is worth saying to whoever asked for the check. And lengths in an .fcm are
 * millimetres by FreeCAD's definition, so a chain read from one answers K5_UNIT_MM from
 * k5_chain_unit, while a chain written to one is scaled into millimetres from whatever unit it was
 * in.
 */

#include "kin5ax.h"

#ifdef __cplusplus
extern "C" {
#endif

/* the longest remark this interface hands over, terminating zero included; a longer one is cut */
#define K5_REMARK_MAX 512

/* What the file cannot say and the conversion has to be told.
 *
 * The two negate flags are the one convention that cannot be read from an .fcm and cannot be guessed
 * from it either: a machine may count a table axis the other way round from the model, and only
 * somebody who has the real machine knows. They are 0 by default, which is right for every file seen
 * so far, and wrong silently when it is not - a part would be cut mirrored.
 *
 * machine_name and template_file_path are owned by the caller and may be NULL or empty. On reading,
 * machine_name names the chain when the file does not; on writing it becomes machine.name.
 * template_file_path on writing is an .fcm whose tool heads, post-processor and other sections are
 * kept as they are, so that a round trip through kin5ax does not throw away what it has no place
 * for; empty writes a minimal file. */
typedef struct k5_fcm_options {
	int negate_table_linear_axes;
	int negate_table_rotary_axes;
	const char *machine_name;
	const char *template_file_path;
} k5_fcm_options;

/* Both flags off and both strings NULL, which is what to start from. */
K5_API void k5_fcm_options_default(k5_fcm_options *out);

/* Reads a FreeCAD machine file into a chain. options may be NULL for the defaults.
 *
 * The chain comes back in millimetres (k5_chain_unit answers K5_UNIT_MM), since that is what an .fcm
 * is, and with the remarks of this conversion on it. It may well not be a five-axis chain the solver
 * can work with - FreeCAD machine files describe three-axis machines too - so ask k5_chain_is_5c
 * before handing it to the solver; the reader puts a remark in either way. */
K5_API k5_status k5_chain_open_fcm(const char *file_path, const k5_fcm_options *options,
	k5_chain *out);

/* Writes the chain as a FreeCAD machine file, scaling its lengths into millimetres. options may be
 * NULL. The remarks of this conversion replace whatever was on the chain. */
K5_API k5_status k5_chain_write_fcm(k5_chain chain, const k5_fcm_options *options,
	const char *file_path);

/* How many remarks the last .fcm conversion this chain took part in left behind - reading or
 * writing, whichever happened last. 0 for a chain that has been through neither, and 0 for a
 * conversion that carried everything across. */
K5_API k5_status k5_fcm_remark_count(k5_chain chain, size_t *out);
/* One of them, cut to capacity; K5_REMARK_MAX is long enough for every remark the bridge writes. */
K5_API k5_status k5_fcm_remark(k5_chain chain, size_t index, char *out, size_t capacity);

#ifdef __cplusplus
}
#endif

#endif /* KIN5AXFCM_H */
