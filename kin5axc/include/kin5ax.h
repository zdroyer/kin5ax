#ifndef KIN5AX_H_INCLUDED
#define KIN5AX_H_INCLUDED

/* kin5ax - analytic inverse kinematics for five-axis milling machines: the C interface.
 *
 * One header, C and C++ clean, no C++ type ever crossing it. Everything is reached through opaque
 * handles; every call returns a k5_status and leaves a message behind for k5_last_error().
 *
 * Lengths are in the unit of the kinematic chain file that was opened, and every file says which:
 * k5_chain_unit answers mm, m or in, and k5_unit_in_millimetres says what to multiply by. Nothing
 * in here converts - a chain's numbers stay in its own unit, and a host scales at its boundary
 * knowing what it scales by. The models shipped with the library are metres. Angles are radians
 * everywhere except where a function says otherwise (k5_gcode_axes::degrees).
 *
 * A handle belongs to one thread at a time: the solver writes the joint values of its chain while it
 * works. Several handles may be used from several threads.
 *
 * Version 1 covers the kinematics and the batch solving of a cutter-location file. The simulation, the
 * FreeCAD (.fcm) bridge and the material model are not here; see the roadmap in the repository.
 */

#include <stddef.h>

#if defined(_WIN32)
#  if defined(KIN5AXC_BUILDING)
#    define K5_API __declspec(dllexport)
#  elif defined(KIN5AXC_STATIC)
#    define K5_API
#  else
#    define K5_API __declspec(dllimport)
#  endif
#else
#  if defined(KIN5AXC_BUILDING)
#    define K5_API __attribute__((visibility("default")))
#  else
#    define K5_API
#  endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* the five joints of a five-axis machine: three linear, two rotary */
#define K5_AXES 5
/* the longest name of a link this interface hands over, terminating zero included */
#define K5_NAME_MAX 64
/* the most cylinders a tool holder of this library is made of (k5_tool_set_holder) */
#define K5_HOLDER_SECTIONS 3

/* ---------------------------------------------------------------- what a call can answer */

typedef enum k5_status {
	K5_OK = 0,
	K5_ERROR_ARGUMENT = 1,			/* a null pointer, an index past the end, a nonsensical dimension */
	K5_ERROR_BUFFER_TOO_SMALL = 2,	/* the count the call needs is written to the out parameter */
	K5_ERROR_FILE = 3,				/* the file cannot be read, or is not what it claims to be */
	K5_ERROR_CHAIN = 4,				/* the chain is not a five-axis chain, or not consistent */
	K5_ERROR_SOLVER = 5,			/* the solver refused the input */
	K5_ERROR_UNKNOWN = 6			/* anything the library did not name */
} k5_status;

/* Why a cutter location has the solutions it has: the stage at which the last candidate was lost.
 * The same values as SOLVE_STATUS of the C++ library. */
typedef enum k5_solve_status {
	K5_SOLVED = 0,
	K5_NO_ORIENTATION,				/* no joint angles put the tool axis where it was asked for */
	K5_NO_POSITION,					/* angles found, but the linear axes reach the point for none of them */
	K5_LIMITS_EXCEEDED,				/* every configuration, and every equivalent of it, is out of limits */
	K5_COLLISION,					/* every configuration within the limits collides */
	K5_SINGULAR_UNSOLVED			/* a location of a singular section the interpolated angle did not solve */
} k5_solve_status;

/* Where a location of a solved path came from. */
typedef enum k5_location_origin {
	K5_FROM_FILE = 0,				/* a cutter location of the file */
	K5_INTERPOLATED,				/* a step invented on the way to one */
	K5_SINGULAR_SECTION			/* from the singular-section solver, which picks its own samples */
} k5_location_origin;

/* How two solids met: what is read between the names of a pair. */
typedef enum k5_collision_kind {
	K5_COLLIDING_PARTS = 0,		/* two solids of the machine: "and" */
	K5_COLLIDING_TOOL,			/* the cutting part of the tool against a solid: "cuts into" */
	K5_COLLIDING_TOOL_SHANK		/* its shank or its holder, which may touch nothing at all: "is in" */
} k5_collision_kind;

typedef enum k5_link_type {
	K5_LINK_PRISMATIC = 0,			/* slides along its own Z */
	K5_LINK_REVOLUTE				/* turns about its own Z */
} k5_link_type;

/* Which branch of the kinematic chain, counted from the machine base outwards. */
typedef enum k5_chain_part {
	K5_PART_TOOL = 0,				/* ends at the tool mount */
	K5_PART_MATERIAL_HOLDER			/* ends at the frame of the material */
} k5_chain_part;

/* The unit every length of a chain file is in. A file names it and a file that does not is refused. */
typedef enum k5_unit {
	K5_UNIT_MM = 0,
	K5_UNIT_M,
	K5_UNIT_IN
} k5_unit;

typedef enum k5_tool_shape {
	K5_TOOL_BALL_NOSE = 0,			/* a ball of radius r at the tip, programmed at its centre */
	K5_TOOL_FLAT_END				/* a cylinder of radius rz, programmed at the tip */
} k5_tool_shape;

/* How the first configuration of a trajectory is chosen; everything after it follows by the shortest
 * path. The start is a convention of the machine and its control, so it has to be said. */
typedef enum k5_start_criterion {
	K5_START_SHORTEST_PATH = 0,		/* not fixed: whatever makes the whole trajectory shortest */
	K5_START_MIN,					/* the smallest value of the first rotary axis */
	K5_START_MAX,					/* the largest value of it */
	K5_START_MIN_ABS,				/* that axis closest to zero: the usual convention of the controls */
	K5_START_RANDOM				/* a candidate drawn from the seed: nobody's convention, say so */
} k5_start_criterion;

/* ---------------------------------------------------------------- the handles */

typedef struct k5_chain_t *k5_chain;	/* a machine */
typedef struct k5_tool_t *k5_tool;		/* a milling tool with its holder */
typedef struct k5_path_t *k5_path;		/* what a batch run produced: the trajectory and its records */

/* The message of the last call that did not return K5_OK, on this thread. Never null; the pointer is
 * valid until the next call on the same thread. */
K5_API const char *k5_last_error(void);

/* The version of this header. k5_version answers with the version of the library actually loaded,
 * which is the one that matters: a host that ships a binding rather than a compiled program only
 * ever sees that one. While the major number is 0 the interface may change between minor versions,
 * so a host should refuse a minor it does not know. */
#define K5_VERSION_MAJOR 0
#define K5_VERSION_MINOR 3
#define K5_VERSION_PATCH 0
#define K5_VERSION_STRING "0.3.0"

K5_API const char *k5_version_string(void);
K5_API void k5_version(int *major, int *minor, int *patch);

/* Which commit this library was built from: "09a933b4f1c2", with "-dirty" after it when the working
 * tree held uncommitted changes, and "unknown" when it was built outside a checkout. A shared
 * library travels on its own - inside a FreeCAD addon, in a release archive - and nothing else about
 * it says where it came from, so a bug report can name the binary exactly.
 *
 * This is build metadata in the sense of semantic versioning: it takes no part in the compatibility
 * of the interface, and two libraries of the same version built from different commits are the same
 * interface. A host checks k5_version; it reports k5_build_string. */
K5_API const char *k5_commit_hash(void);
/* "0.1.0+09a933b4f1c2": the version and the commit in one string, which is what to put in a report */
K5_API const char *k5_build_string(void);

/* ---------------------------------------------------------------- the settings of a run */

typedef struct k5_settings {
	int check_limits;					/* 1: a configuration out of its joint limits is no solution */
	int check_collisions;				/* 1: a configuration whose parts meet is no solution. Needs a tool */
	int interpolate;					/* 1: invent locations between two of the file, as dense as below */
	double max_position_difference;		/* no interpolated step longer than this; 0 switches the test off */
	double max_orientation_difference_degrees;	/* nor turning the tool axis further than this */
	double angle_distance_weight;		/* what a radian of a rotary axis counts for in the path length */
	double displacement_weight;			/* and what a length unit of a linear axis counts for */
	int start_criterion;				/* k5_start_criterion */
	int start_count_from_tool;			/* 1: the rotary axis nearest the tool; 0: the one nearest the part */
	unsigned int start_random_seed;		/* K5_START_RANDOM only, so that a run can be repeated */
} k5_settings;

/* The settings the library recommends: both checks on, interpolation on at 1E-2 of a length unit and
 * one degree, and the start of the trajectory at the rotary axis nearest the tool, closest to zero. */
K5_API void k5_settings_default(k5_settings *out);

/* ---------------------------------------------------------------- a machine */

/* Reads a .kc (XML) or .kc.json file; the format is told by the content, not by the name. */
K5_API k5_status k5_chain_open(const char *file_path, k5_chain *out);
K5_API void k5_chain_free(k5_chain chain);

/* The name the file gives the model, cut to K5_NAME_MAX; an empty string when it has none. */
K5_API k5_status k5_chain_name(k5_chain chain, char *out, size_t capacity);
/* 1 when this is a five-axis chain the solver can work with: three linear and two rotary joints. */
K5_API k5_status k5_chain_is_5c(k5_chain chain, int *out);
/* The unit of every length of this chain, as its file declared it: a k5_unit. */
K5_API k5_status k5_chain_unit(k5_chain chain, int *out);
/* How many millimetres one of that unit is: 1, 1000 or 25.4. K5_ERROR_ARGUMENT for any other value,
 * so a host that passes what k5_chain_unit gave it can multiply without a table of its own. */
K5_API k5_status k5_unit_in_millimetres(int unit, double *out);
/* The joints of the machine, the terminals of the two branches excluded. Five for a 5C machine. */
K5_API k5_status k5_chain_link_count(k5_chain chain, size_t *out);

typedef struct k5_link {
	int type;							/* k5_link_type */
	int constrained;					/* 1 when the two limits below apply */
	double minimum;						/* the joint limits, in the unit of the file or in radians */
	double maximum;
	double value;						/* where the joint stands at the moment */
	char name[K5_NAME_MAX];				/* the name from the file, cut to fit */
} k5_link;

/* The joint of index 0..k5_chain_link_count()-1. The order runs from the material terminal through the
 * base to the tool terminal, which is also the order of the values in every q[K5_AXES] of this
 * interface. */
K5_API k5_status k5_chain_link(k5_chain chain, size_t index, k5_link *out);

/* Which value of a q[K5_AXES] is the first rotary axis met walking the chain from that end: from the
 * tool end the axis nearest the tool, from the material end the one nearest the part. */
K5_API k5_status k5_chain_first_rotary_axis(k5_chain chain, int part, size_t *out);

/* The frames of the segments of one branch at the joint values the chain stands at, in the frame of the
 * machine base: 32 doubles per segment, two 4x4 matrices. First the frame the collision boxes of the
 * segment are given in and whose Z is the axis of its joint, then the frame after the joint motion,
 * which is the frame the next segment sits in. The last frame of the tool branch is the tool mount, of
 * the material branch the frame of the material.
 *
 * Each matrix is the layout OpenGL and glTF use, so it can be handed to either as it is: the three
 * basis vectors of the frame at 0..2, 4..6 and 8..10, the origin of the frame at 12..14, and 0,0,0,1
 * at 3, 7, 11 and 15.
 *
 * out may be null to ask for the count only; count_out gets the number of segments either way. */
K5_API k5_status k5_chain_frames(k5_chain chain, int part, double *out, size_t capacity_in_doubles,
	size_t *count_out);

/* The collision boxes of the segment of that index in that branch - the segments being those of
 * k5_chain_frames, the terminal included - in the first of the two frames k5_chain_frames gives for
 * the segment: 6 doubles per box, the minimum corner x y z then the maximum corner x y z. A segment
 * may have no box at all; the models shipped with the library carry boxes on their terminals only.
 * out may be null to ask for the count only; count_out gets the number of boxes either way. */
K5_API k5_status k5_chain_segment_boxes(k5_chain chain, int part, size_t index, double *out,
	size_t capacity_in_doubles, size_t *count_out);

/* Puts the chain into a set of joint values (K5_AXES of them, in the order of k5_chain_link) and
 * nothing more: what k5_chain_frames and k5_chain_link answer afterwards is for that configuration.
 * The joint limits are not checked - a host drawing the machine may want to see it past them. */
K5_API k5_status k5_chain_set_configuration(k5_chain chain, const double *q);

typedef struct k5_collision {
	char first[K5_NAME_MAX];	/* the two solids that met, named as the chain file names them, cut to fit */
	char second[K5_NAME_MAX];
	int kind;					/* k5_collision_kind */
	size_t locations;			/* at how many locations of the path this pair met */
} k5_collision;

/* The name a report gives one solid of a segment of a branch - the name of the segment, or its place
 * in the branch when the file gives it none, and the number of the box when the segment has more than
 * one. A host that draws a machine needs these to show which solids k5_path_collision names. */
K5_API k5_status k5_chain_solid_name(k5_chain chain, int part, size_t segment, size_t box,
	char *out, size_t capacity);

/* Checks the chain, at the configuration it stands in, against the tool: collides gets 1 when
 * anything met. What met is then k5_chain_collision_count and k5_chain_collision, until the next
 * check on this chain - a solve makes its own checks, so ask before solving anything else. */
K5_API k5_status k5_chain_check_collisions(k5_chain chain, k5_tool tool, int *collides);
K5_API k5_status k5_chain_collision_count(k5_chain chain, size_t *out);
K5_API k5_status k5_chain_collision(k5_chain chain, size_t index, k5_collision *out);

/* Puts the chain into a set of joint values (K5_AXES of them, in the order of k5_chain_link) and
 * answers where the programmed point of the tool ends up: position[3] and the tool axis axis[3], in the
 * frame of the material. The forward kinematics, and the way to check k5_solve_location.
 * tool may be null, and then the chain keeps the tool assembly it was last given. */
K5_API k5_status k5_chain_tool_point(k5_chain chain, k5_tool tool, const double *q,
	double *position, double *axis);

/* ---------------------------------------------------------------- the workpiece holder */

/* The vice, the chuck, the plate the part is bolted to: where the part sits on the terminal of the
 * workpiece branch, which is where a holder is mounted on every machine.
 *
 * It is a frame and not a solid: 16 doubles in the layout of k5_chain_frames, the transformation
 * from the frame a cutter location is given in - the frame of the part - to that terminal. Lengths
 * in the unit of the chain file. null is the identity, which is the part sitting on the table with
 * nothing in between and is what a chain starts with.
 *
 * Everything this interface answers about a location is in the frame of the part: k5_solve_location
 * and the two postprocessing calls take one there, k5_chain_tool_point answers there. The frames of
 * k5_chain_frames are the machine's own and carry no holder, so a host that draws the part puts it
 * in the workpiece terminal by this transformation.
 *
 * The holder is the mirror image of the gauge length of a tool: the one measurement that says where
 * the work is, as the gauge length says where the tool is. It is refused when it is not rigid - a
 * frame that scales or shears is not a way of holding anything. */
K5_API k5_status k5_chain_set_workpiece_holder(k5_chain chain, const double *transformation);
/* The holder that was given, or the identity: 16 doubles in the same layout. */
K5_API k5_status k5_chain_workpiece_holder(k5_chain chain, double *out);

/* ---------------------------------------------------------------- a tool */

/* The tool and its holder, all lengths in the unit of the chain file:
 *   rz                 the radius of the shank, and of a flat end mill
 *   r                  the radius of the ball of a ball nose tool (ignored by a flat end mill)
 *   cutting_height     how far the cutting part reaches up from the tip
 *   shank_length       the part above it that must cut nothing and meet nothing
 *   holder_length      the holder hanging from the tool mount of the machine
 *   holder_radius      its radius; without one the holder keeps its length but has no body
 *   gauge_length       from the tip to the tool mount: the tool length a control is set with
 *
 * THE GAUGE LENGTH is the measurement that ties the tool to the machine. The end of the tool branch
 * of a chain is where a holder is clamped, for every machine and every tool, so how far the tip
 * reaches below it is a property of what is in the spindle and of nothing else. It is measured on a
 * presetter, not added up from the dimensions above: the same tool sits at different heights
 * depending on how far it is pulled out of its holder. Pass 0 for the tool standing right up against
 * its holder, which is cutting_height + shank_length + holder_length and is a convenience rather
 * than a measurement.
 *
 * The programmed point follows the shape: the centre of the ball, or the tip of a flat end mill.
 * Dimensions that do not make a tool are refused with K5_ERROR_ARGUMENT and a message that says why. */
K5_API k5_status k5_tool_create(int shape, double rz, double r, double cutting_height,
	double shank_length, double holder_length, double holder_radius, double gauge_length,
	k5_tool *out);
K5_API void k5_tool_free(k5_tool tool);

/* From the tip of the tool to the tool mount of the machine: the cutting part, the shank and the holder. */
K5_API k5_status k5_tool_assembly_length(k5_tool tool, double *out);
/* How far below the tool mount the programmed point hangs, which is what the solver solves for. */
K5_API k5_status k5_tool_mount_to_point_distance(k5_tool tool, double *out);
/* From the tip to the tool mount: what was given to k5_tool_create, or the derived length when 0 was. */
K5_API k5_status k5_tool_gauge_length(k5_tool tool, double *out);

/* ---------------------------------------------------------------- tool holders */

/* A holder hangs from the tool mount of the machine, which is the terminal of the tool branch of
 * every chain, and the tool is clamped in it. Its shape is up to K5_HOLDER_SECTIONS cylinders given
 * from the mount downwards - the taper, whatever it narrows to, the nose the tool comes out of -
 * which is enough to find out whether the holder meets a part of the machine or ploughs through
 * material. Its model is the designation on the holder, free text: a host matches it against the
 * holders it has of its own, and this library never reads it.
 *
 * A holder is never a tool length. However long the holder is, the tip of the tool reaches the gauge
 * length of k5_tool_create below the mount. */

/* The holder the chain file says its machine usually carries: the designation, cut to fit, and an
 * empty string when the file names none. It is a default for a host to offer, nothing more. */
K5_API k5_status k5_chain_default_holder_model(k5_chain chain, char *out, size_t capacity);

/* The shape of that holder, in the unit of the chain file: 2 doubles per cylinder, the length then
 * the radius, from the mount downwards. A radius of zero is a cylinder without a body, a distance
 * only. A holder may have no cylinder at all, which is a name for the host to resolve.
 * out may be null to ask for the count only; count_out gets the number of cylinders either way. */
K5_API k5_status k5_chain_default_holder(k5_chain chain, double *out, size_t capacity_in_doubles,
	size_t *count_out);

/* The holders a five-axis machine usually carries, to be offered in a list instead of measured. The
 * dimensions are the usual ones for each taper, near enough for a collision check and no substitute
 * for measuring the holder on the machine; they are in METRES, whatever unit a chain is in. */
K5_API k5_status k5_tool_holder_catalogue_size(size_t *out);
/* The designation of the holder at that place in the catalogue, cut to fit. */
K5_API k5_status k5_tool_holder_catalogue_model(size_t index, char *out, size_t capacity);
/* Its cylinders, as k5_chain_default_holder gives them. */
K5_API k5_status k5_tool_holder_catalogue(size_t index, double *out, size_t capacity_in_doubles,
	size_t *count_out);

/* Gives the tool a holder of several cylinders, instead of the single one k5_tool_create was given:
 * sections is 2 doubles per cylinder as above, section_count of them, and may be null for a holder
 * with no body. The lengths are in the unit the tool was made in. When the tool was created with a
 * gauge length nothing moves, which is the point of measuring one; when it was created with 0 the
 * tool stands against its holder, so the tip moves with it. */
K5_API k5_status k5_tool_set_holder(k5_tool tool, const double *sections, size_t section_count);

/* ---------------------------------------------------------------- one cutter location */

/* Every configuration that puts the programmed point of the tool at position[3] with the tool axis
 * along axis[3] (both in the frame of the material), is within the joint limits and does not collide.
 *
 * q_out takes capacity_in_configurations * K5_AXES doubles, one configuration after another in the
 * order of k5_chain_link. count_out gets the number of configurations found - call with q_out null to
 * ask for it first. status_out, when not null, gets the k5_solve_status that says why the answer is
 * empty. A location with no solution is not an error: the call returns K5_OK with a count of 0.
 *
 * tool may be null when the settings do not check collisions. */
K5_API k5_status k5_solve_location(k5_chain chain, k5_tool tool, const k5_settings *settings,
	const double *position, const double *axis,
	double *q_out, size_t capacity_in_configurations, size_t *count_out, int *status_out);

/* ---------------------------------------------------------------- a whole path */

/* Reads a cutter-location file (GOTO/x,y,z,i,j,k lines), solves every location and the locations
 * interpolated between them, and picks the trajectory: the shortest one, starting where the settings
 * say. The result is a k5_path to be read with the calls below and released with k5_path_free.
 * tool may be null when the settings do not check collisions. */
K5_API k5_status k5_postprocess_file(k5_chain chain, k5_tool tool, const k5_settings *settings,
	const char *cl_file_path, k5_path *out);

/* The same for a tool path the host has in memory and no file to put it in - an agent that generates
 * one, a CAM system that holds it, a path that was filtered: count locations, their programmed points
 * in positions[3*count] and their tool axes in axes[3*count], both in the frame of the material. The
 * axes need not be unit vectors, but an axis of length zero is no direction and is refused with the
 * number of the location. count may be 0, and then the path holds nothing.
 *
 * Everything else is as k5_postprocess_file, and the records of the result count the locations the way
 * the lines of a file are counted: the first location is 1. */
K5_API k5_status k5_postprocess_locations(k5_chain chain, k5_tool tool, const k5_settings *settings,
	const double *positions, const double *axes, size_t count, k5_path *out);

K5_API void k5_path_free(k5_path path);

/* How many locations the path holds: those of the file and those invented between them. */
K5_API k5_status k5_path_size(k5_path path, size_t *out);

typedef struct k5_location {
	int status;							/* k5_solve_status */
	int origin;							/* k5_location_origin */
	size_t file_location;				/* the cutter location of the file this one belongs to, from 1:
										 * its own number, or the number of the one before it for
										 * anything interpolated on the way to it */
	size_t file_line;					/* the line of the file that cutter location was read from, from 1 */
	int has_solution;					/* 1 when q below is the configuration chosen for this location */
	double q[K5_AXES];					/* in the order of k5_chain_link */
	double position[3];					/* the programmed point of the tool this location was solved for, and */
	double axis[3];						/* its tool axis: the cutter location's own, or the ones invented on the
										 * way to it. Filled whether or not the location has a solution: they
										 * say where the tool was asked to be */
} k5_location;

K5_API k5_status k5_path_location(k5_path path, size_t index, k5_location *out);
/* How many locations have that k5_solve_status / that k5_location_origin. */
K5_API k5_status k5_path_count_of_status(k5_path path, int status, size_t *out);
K5_API k5_status k5_path_count_of_origin(k5_path path, int origin, size_t *out);
/* The biggest errors of the solutions of this run: how far a chosen configuration put the tool point
 * from where it was asked for, and the length of the error of the tool axis. Below a nanometre on the
 * machines the library is tested with. */
K5_API k5_status k5_path_errors(k5_path path, double *max_position_error, double *max_orientation_error);
/* Which solids of the machine met while this path was being solved, and at how many of its locations
 * each pair did - a location whose every configuration was rejected by the same pair counts once.
 * Empty when the collision check was off or nothing met; the pairs come in no particular order.
 * A location counted here has no solution for that reason: these are the K5_COLLISION ones. */
K5_API k5_status k5_path_collision_count(k5_path path, size_t *out);
K5_API k5_status k5_path_collision(k5_path path, size_t index, k5_collision *out);



/* Where the machine is left standing: the configuration chosen for the last location that has one.
 * Hand it back to the next run and the trajectory carries on from there (not in version 1 - the
 * settings have no hint field yet; k5_path_last_configuration is the half that exists). */
K5_API k5_status k5_path_last_configuration(k5_path path, double *q);

typedef struct k5_gcode_axes {
	size_t x, y, z;						/* which value of a q[K5_AXES] is which linear axis */
	size_t angle1, angle2;				/* and which are the two rotary ones */
	char angle1_letter;					/* the letter the control calls it by, 'A', 'B' or 'C': the chain
										 * file does not carry it yet, so the host says it */
	char angle2_letter;
	int degrees;						/* 1: write the angles in degrees, 0: in radians */
} k5_gcode_axes;

/* The indices of the five axes of this machine, taken from the structure of the chain. The two letters
 * come out as 'A' and 'C' - a guess the host is expected to correct, because nothing in the file says
 * what the control calls its axes. degrees is 1. */
K5_API k5_status k5_chain_default_gcode_axes(k5_chain chain, k5_gcode_axes *out);

/* Writes the trajectory as G-code: one N.. G1 X.. Y.. Z.. A.. C.. line per location that has a
 * solution, with the decimal point whatever the locale of the process. Feed rates, tool changes and
 * machine words are not written. */
K5_API k5_status k5_path_write_gcode(k5_path path, const k5_gcode_axes *axes, const char *file_path);

/* ---------------------------------------------------------------- the cutting simulation */

/* A block of material to cut, and a simulation that cuts it with a solved path. Rudimentary, and
 * said so: the material is a box of voxels, the tool is stamped into it once per location of the
 * path with nothing swept between two locations, and there is no feed or time. Dense interpolation
 * - steps no longer than a voxel - is what makes it look like a cut. What it is good for is the
 * geometry: whether the trajectory the solver chose takes the material off where the path meant
 * it to, and whether the shank or the holder of the tool went through material that was still
 * there, which is a gouge. */
typedef struct k5_material_t *k5_material;
typedef struct k5_simulation_t *k5_simulation;

/* A box of material in the frame of the material of the chain, all lengths in the unit of the chain
 * file: its size, the side of one voxel, and the corner it starts at. Everything is material to
 * begin with. The voxel count is the size over the voxel side along each axis, rounded up; keep it
 * in the millions, not the billions. */
K5_API k5_status k5_material_create(double x_size, double y_size, double z_size, double voxel_size,
	double x_start, double y_start, double z_start, k5_material *out);
K5_API void k5_material_free(k5_material material);
/* Whole again, as it was created. */
K5_API k5_status k5_material_reset(k5_material material);
/* How many voxels there are along X, Y and Z. */
K5_API k5_status k5_material_dimensions(k5_material material, size_t *nx, size_t *ny, size_t *nz);
/* The volume of the block and the volume removed so far, whole voxels, in the unit of the file cubed. */
K5_API k5_status k5_material_volumes(k5_material material, double *total, double *removed);
/* The surface of what is left: the voxels that are still material and have a removed voxel or the
 * outside next to them, 3 doubles each - the corner of the voxel nearest the start of the block, in
 * the frame of the material. What a host draws, as points or as cubes of the voxel side. out may be
 * null to ask for the count only; count_out gets the number of voxels either way. */
K5_API k5_status k5_material_surface(k5_material material, double *out, size_t capacity_in_doubles,
	size_t *count_out);
/* The same surface as a solid: the exposed faces of what is left - every side of a material voxel
 * with a removed voxel or the outside behind it - as two triangles each, wound to face outwards, 18
 * doubles per face (x y z of three corners, twice). A face a renderer cannot see from any side is
 * not among them. out may be null to ask for the count of faces only; count_out gets it either way,
 * and the doubles needed are 18 times that. */
K5_API k5_status k5_material_surface_triangles(k5_material material, double *out,
	size_t capacity_in_doubles, size_t *count_out);

/* One cut, with no simulation and no machine: the tool put at position[3] with its axis along
 * axis[3], both in the frame of the material - the programmed point of the tool, which is the centre
 * of the ball of a ball nose one - and stamped into the material. gouged, when not null, gets 1 when
 * the shank or the holder is in material that is still there after the cut, and gouge_point[3] where.
 * What a host needs for a location its simulation will not cut: one the machine cannot reach, which
 * is still a place the tool was asked to be. */
K5_API k5_status k5_material_cut(k5_material material, k5_tool tool, const double *position,
	const double *axis, int *gouged, double *gouge_point);

/* A simulation of the path cutting the material with that tool on that chain. The four belong to the
 * caller and must outlive the simulation; the chain's joint values and the tool's position are
 * changed by every step. The chain is given the tool as its assembly, as a solve gives it. */
K5_API k5_status k5_simulation_create(k5_chain chain, k5_tool tool, k5_path path, k5_material material,
	k5_simulation *out);
/* The same with no machine at all: the path as a host has it - count locations, programmed points in
 * positions[3*count] and tool axes in axes[3*count], in the frame of the material, which is the frame
 * of the block - made as dense as the settings ask (interpolate, max_position_difference,
 * max_orientation_difference_degrees; the rest of the settings mean nothing here) and the tool put at
 * every location of the dense path and stamped in. The tool and the material belong to the caller and
 * must outlive the simulation. An axis of length zero is refused with the number of the location. */
K5_API k5_status k5_simulation_create_from_locations(k5_tool tool, const k5_settings *settings,
	const double *positions, const double *axes, size_t count, k5_material material, k5_simulation *out);
K5_API void k5_simulation_free(k5_simulation simulation);
/* The location of that index of what the steps walk: where the programmed point of the tool goes
 * (position[3]) and which way the tool points (axis[3]), in the frame of the material; source gets
 * the index of the location it was made from - its own, or for one invented on the way to the next,
 * the one before it - and invented 1 for such a one. Any of the four may be null. For a simulation on
 * a chain the position and the axis are the tool point of the configuration chosen for the location,
 * and a location without a solution is refused with K5_ERROR_ARGUMENT. */
K5_API k5_status k5_simulation_location(k5_simulation simulation, size_t index, double *position,
	double *axis, size_t *source, int *invented);
/* Back to the first location of the path. The material is not touched: k5_material_reset is the
 * caller's decision, since one block may be cut by several paths in turn. */
K5_API k5_status k5_simulation_reset(k5_simulation simulation);
/* The next location that has a solution: the chain takes its configuration, the tool its frame, the
 * material is cut. stepped gets 1, or 0 when the path is exhausted and nothing was done. */
K5_API k5_status k5_simulation_step(k5_simulation simulation, int *stepped);
/* All the remaining locations. */
K5_API k5_status k5_simulation_run(k5_simulation simulation);
/* How far the simulation is: the locations passed so far (those without a solution are passed without
 * a cut), how many the path has, and whether it is finished. Any of the three may be null. */
K5_API k5_status k5_simulation_progress(k5_simulation simulation, size_t *step_index, size_t *step_count,
	int *finished);
/* The gouges so far: locations where the shank or the holder of the tool met material that was still
 * there. A gouge does not stop the simulation. first_step and first_point[3] (in the frame of the
 * material) are those of the first gouge and mean nothing while count is 0; either may be null. */
K5_API k5_status k5_simulation_gouges(k5_simulation simulation, size_t *count, size_t *first_step,
	double *first_point);

#ifdef __cplusplus
}
#endif

#endif /* KIN5AX_H_INCLUDED */
