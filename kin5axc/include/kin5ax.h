#ifndef KIN5AX_H_INCLUDED
#define KIN5AX_H_INCLUDED

/* kin5ax - analytic inverse kinematics for five-axis milling machines: the C interface.
 *
 * One header, C and C++ clean, no C++ type ever crossing it. Everything is reached through opaque
 * handles; every call returns a k5_status and leaves a message behind for k5_last_error().
 *
 * Lengths are in the unit of the kinematic chain file that was opened - metres for the models shipped
 * with the library. The files do not carry their unit yet; when they do, this header will say
 * millimetres and convert at the boundary. Angles are radians everywhere except where a function says
 * otherwise (k5_gcode_axes::degrees).
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

typedef enum k5_link_type {
	K5_LINK_PRISMATIC = 0,			/* slides along its own Z */
	K5_LINK_REVOLUTE				/* turns about its own Z */
} k5_link_type;

/* Which branch of the kinematic chain, counted from the machine base outwards. */
typedef enum k5_chain_part {
	K5_PART_TOOL = 0,				/* ends at the tool mount */
	K5_PART_MATERIAL_HOLDER			/* ends at the frame of the material */
} k5_chain_part;

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

K5_API const char *k5_version_string(void);
K5_API void k5_version(int *major, int *minor, int *patch);

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

/* Puts the chain into a set of joint values (K5_AXES of them, in the order of k5_chain_link) and
 * answers where the programmed point of the tool ends up: position[3] and the tool axis axis[3], in the
 * frame of the material. The forward kinematics, and the way to check k5_solve_location.
 * tool may be null, and then the chain keeps the tool assembly it was last given. */
K5_API k5_status k5_chain_tool_point(k5_chain chain, k5_tool tool, const double *q,
	double *position, double *axis);

/* ---------------------------------------------------------------- a tool */

/* The tool and its holder, all lengths in the unit of the chain file:
 *   rz                 the radius of the shank, and of a flat end mill
 *   r                  the radius of the ball of a ball nose tool (ignored by a flat end mill)
 *   cutting_height     how far the cutting part reaches up from the tip
 *   shank_length       the part above it that must cut nothing and meet nothing
 *   holder_length      the holder between the tool and the tool mount of the machine
 *   holder_radius      its radius; without one the holder keeps its length but has no body
 * The programmed point follows the shape: the centre of the ball, or the tip of a flat end mill.
 * Dimensions that do not make a tool are refused with K5_ERROR_ARGUMENT and a message that says why. */
K5_API k5_status k5_tool_create(int shape, double rz, double r, double cutting_height,
	double shank_length, double holder_length, double holder_radius, k5_tool *out);
K5_API void k5_tool_free(k5_tool tool);

/* From the tip of the tool to the tool mount of the machine: the cutting part, the shank and the holder. */
K5_API k5_status k5_tool_assembly_length(k5_tool tool, double *out);
/* How far below the tool mount the programmed point hangs, which is what the solver solves for. */
K5_API k5_status k5_tool_mount_to_point_distance(k5_tool tool, double *out);

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
} k5_location;

K5_API k5_status k5_path_location(k5_path path, size_t index, k5_location *out);
/* How many locations have that k5_solve_status / that k5_location_origin. */
K5_API k5_status k5_path_count_of_status(k5_path path, int status, size_t *out);
K5_API k5_status k5_path_count_of_origin(k5_path path, int origin, size_t *out);
/* The biggest errors of the solutions of this run: how far a chosen configuration put the tool point
 * from where it was asked for, and the length of the error of the tool axis. Below a nanometre on the
 * machines the library is tested with. */
K5_API k5_status k5_path_errors(k5_path path, double *max_position_error, double *max_orientation_error);
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

#ifdef __cplusplus
}
#endif

#endif /* KIN5AX_H_INCLUDED */
