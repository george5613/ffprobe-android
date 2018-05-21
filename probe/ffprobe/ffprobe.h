//
// Created by George Ling on 2017/10/25.
//

#ifndef FFPROBE_FFPROBE_H
#define FFPROBE_FFPROBE_H

#include <libavutil/rational.h>
#include <libavutil/log.h>
#include <libavutil/bprint.h>

/* FFprobe context */
const char *input_filename;
static AVInputFormat *iformat = NULL;

static struct AVHashContext *hash;

static const struct {
    double bin_val;
    double dec_val;
    const char *bin_str;
    const char *dec_str;
} si_prefixes[] = {
        { 1.0, 1.0, "", "" },
        { 1.024e3, 1e3, "Ki", "K" },
        { 1.048576e6, 1e6, "Mi", "M" },
        { 1.073741824e9, 1e9, "Gi", "G" },
        { 1.099511627776e12, 1e12, "Ti", "T" },
        { 1.125899906842624e15, 1e15, "Pi", "P" },
};

static const char unit_second_str[]         = "s"    ;
static const char unit_hertz_str[]          = "Hz"   ;
static const char unit_byte_str[]           = "byte" ;
static const char unit_bit_per_second_str[] = "bit/s";

static int nb_streams;
static uint64_t *nb_streams_packets;
static uint64_t *nb_streams_frames;
static int *selected_streams;

typedef struct LogBuffer {
    char *context_name;
    int log_level;
    char *log_message;
    AVClassCategory category;
    char *parent_name;
    AVClassCategory parent_category;
}LogBuffer;

static LogBuffer *log_buffer;
static int log_buffer_size;

//LOG
#if HAVE_THREADS
pthread_mutex_t log_mutex;
#endif
static void log_callback(void *ptr, int level, const char *fmt, va_list vl);

//SECTION

/* section structure definition */

#define SECTION_MAX_NB_CHILDREN 10

struct section {
    int id;             ///< unique id identifying a section
    const char *name;

#define SECTION_FLAG_IS_WRAPPER      1 ///< the section only contains other sections, but has no data at its own level
#define SECTION_FLAG_IS_ARRAY        2 ///< the section contains an array of elements of the same type
#define SECTION_FLAG_HAS_VARIABLE_FIELDS 4 ///< the section may contain a variable number of fields with variable keys.
    ///  For these sections the element_name field is mandatory.
    int flags;
    int children_ids[SECTION_MAX_NB_CHILDREN+1]; ///< list of children section IDS, terminated by -1
    const char *element_name; ///< name of the contained element, if provided
    const char *unique_name;  ///< unique section name, in case the name is ambiguous
    AVDictionary *entries_to_show;
    int show_all_entries;
};

typedef enum {
    SECTION_ID_NONE = -1,
    SECTION_ID_CHAPTER,
    SECTION_ID_CHAPTER_TAGS,
    SECTION_ID_CHAPTERS,
    SECTION_ID_ERROR,
    SECTION_ID_FORMAT,
    SECTION_ID_FORMAT_TAGS,
    SECTION_ID_FRAME,
    SECTION_ID_FRAMES,
    SECTION_ID_FRAME_TAGS,
    SECTION_ID_FRAME_SIDE_DATA_LIST,
    SECTION_ID_FRAME_SIDE_DATA,
    SECTION_ID_FRAME_LOG,
    SECTION_ID_FRAME_LOGS,
    SECTION_ID_LIBRARY_VERSION,
    SECTION_ID_LIBRARY_VERSIONS,
    SECTION_ID_PACKET,
    SECTION_ID_PACKET_TAGS,
    SECTION_ID_PACKETS,
    SECTION_ID_PACKETS_AND_FRAMES,
    SECTION_ID_PACKET_SIDE_DATA_LIST,
    SECTION_ID_PACKET_SIDE_DATA,
    SECTION_ID_PIXEL_FORMAT,
    SECTION_ID_PIXEL_FORMAT_FLAGS,
    SECTION_ID_PIXEL_FORMAT_COMPONENT,
    SECTION_ID_PIXEL_FORMAT_COMPONENTS,
    SECTION_ID_PIXEL_FORMATS,
    SECTION_ID_PROGRAM_STREAM_DISPOSITION,
    SECTION_ID_PROGRAM_STREAM_TAGS,
    SECTION_ID_PROGRAM,
    SECTION_ID_PROGRAM_STREAMS,
    SECTION_ID_PROGRAM_STREAM,
    SECTION_ID_PROGRAM_TAGS,
    SECTION_ID_PROGRAM_VERSION,
    SECTION_ID_PROGRAMS,
    SECTION_ID_ROOT,
    SECTION_ID_STREAM,
    SECTION_ID_STREAM_DISPOSITION,
    SECTION_ID_STREAMS,
    SECTION_ID_STREAM_TAGS,
    SECTION_ID_STREAM_SIDE_DATA_LIST,
    SECTION_ID_STREAM_SIDE_DATA,
    SECTION_ID_SUBTITLE,
} SectionID;

static inline void mark_section_show_entries(SectionID section_id,
                                             int show_all_entries, AVDictionary *entries);

#define DEFINE_OPT_SHOW_SECTION(section, target_section_id)             \
    static int opt_show_##section(const char *opt, const char *arg)     \
    {                                                                   \
        mark_section_show_entries(SECTION_ID_##target_section_id, 1, NULL); \
        return 0;                                                       \
    }

DEFINE_OPT_SHOW_SECTION(chapters,         CHAPTERS)
DEFINE_OPT_SHOW_SECTION(error,            ERROR)
DEFINE_OPT_SHOW_SECTION(format,           FORMAT)
DEFINE_OPT_SHOW_SECTION(frames,           FRAMES)
DEFINE_OPT_SHOW_SECTION(library_versions, LIBRARY_VERSIONS)
DEFINE_OPT_SHOW_SECTION(packets,          PACKETS)
DEFINE_OPT_SHOW_SECTION(pixel_formats,    PIXEL_FORMATS)
DEFINE_OPT_SHOW_SECTION(program_version,  PROGRAM_VERSION)
DEFINE_OPT_SHOW_SECTION(streams,          STREAMS)
DEFINE_OPT_SHOW_SECTION(programs,         PROGRAMS)

static struct section sections[] = {
        [SECTION_ID_CHAPTERS] =           { SECTION_ID_CHAPTERS, "chapters", SECTION_FLAG_IS_ARRAY, { SECTION_ID_CHAPTER, -1 } },
        [SECTION_ID_CHAPTER] =            { SECTION_ID_CHAPTER, "chapter", 0, { SECTION_ID_CHAPTER_TAGS, -1 } },
        [SECTION_ID_CHAPTER_TAGS] =       { SECTION_ID_CHAPTER_TAGS, "tags", SECTION_FLAG_HAS_VARIABLE_FIELDS, { -1 }, .element_name = "tag", .unique_name = "chapter_tags" },
        [SECTION_ID_ERROR] =              { SECTION_ID_ERROR, "error", 0, { -1 } },
        [SECTION_ID_FORMAT] =             { SECTION_ID_FORMAT, "format", 0, { SECTION_ID_FORMAT_TAGS, -1 } },
        [SECTION_ID_FORMAT_TAGS] =        { SECTION_ID_FORMAT_TAGS, "tags", SECTION_FLAG_HAS_VARIABLE_FIELDS, { -1 }, .element_name = "tag", .unique_name = "format_tags" },
        [SECTION_ID_FRAMES] =             { SECTION_ID_FRAMES, "frames", SECTION_FLAG_IS_ARRAY, { SECTION_ID_FRAME, SECTION_ID_SUBTITLE, -1 } },
        [SECTION_ID_FRAME] =              { SECTION_ID_FRAME, "frame", 0, { SECTION_ID_FRAME_TAGS, SECTION_ID_FRAME_SIDE_DATA_LIST, SECTION_ID_FRAME_LOGS, -1 } },
        [SECTION_ID_FRAME_TAGS] =         { SECTION_ID_FRAME_TAGS, "tags", SECTION_FLAG_HAS_VARIABLE_FIELDS, { -1 }, .element_name = "tag", .unique_name = "frame_tags" },
        [SECTION_ID_FRAME_SIDE_DATA_LIST] ={ SECTION_ID_FRAME_SIDE_DATA_LIST, "side_data_list", SECTION_FLAG_IS_ARRAY, { SECTION_ID_FRAME_SIDE_DATA, -1 }, .element_name = "side_data", .unique_name = "frame_side_data_list" },
        [SECTION_ID_FRAME_SIDE_DATA] =     { SECTION_ID_FRAME_SIDE_DATA, "side_data", 0, { -1 } },
        [SECTION_ID_FRAME_LOGS] =         { SECTION_ID_FRAME_LOGS, "logs", SECTION_FLAG_IS_ARRAY, { SECTION_ID_FRAME_LOG, -1 } },
        [SECTION_ID_FRAME_LOG] =          { SECTION_ID_FRAME_LOG, "log", 0, { -1 },  },
        [SECTION_ID_LIBRARY_VERSIONS] =   { SECTION_ID_LIBRARY_VERSIONS, "library_versions", SECTION_FLAG_IS_ARRAY, { SECTION_ID_LIBRARY_VERSION, -1 } },
        [SECTION_ID_LIBRARY_VERSION] =    { SECTION_ID_LIBRARY_VERSION, "library_version", 0, { -1 } },
        [SECTION_ID_PACKETS] =            { SECTION_ID_PACKETS, "packets", SECTION_FLAG_IS_ARRAY, { SECTION_ID_PACKET, -1} },
        [SECTION_ID_PACKETS_AND_FRAMES] = { SECTION_ID_PACKETS_AND_FRAMES, "packets_and_frames", SECTION_FLAG_IS_ARRAY, { SECTION_ID_PACKET, -1} },
        [SECTION_ID_PACKET] =             { SECTION_ID_PACKET, "packet", 0, { SECTION_ID_PACKET_TAGS, SECTION_ID_PACKET_SIDE_DATA_LIST, -1 } },
        [SECTION_ID_PACKET_TAGS] =        { SECTION_ID_PACKET_TAGS, "tags", SECTION_FLAG_HAS_VARIABLE_FIELDS, { -1 }, .element_name = "tag", .unique_name = "packet_tags" },
        [SECTION_ID_PACKET_SIDE_DATA_LIST] ={ SECTION_ID_PACKET_SIDE_DATA_LIST, "side_data_list", SECTION_FLAG_IS_ARRAY, { SECTION_ID_PACKET_SIDE_DATA, -1 }, .element_name = "side_data", .unique_name = "packet_side_data_list" },
        [SECTION_ID_PACKET_SIDE_DATA] =     { SECTION_ID_PACKET_SIDE_DATA, "side_data", 0, { -1 } },
        [SECTION_ID_PIXEL_FORMATS] =      { SECTION_ID_PIXEL_FORMATS, "pixel_formats", SECTION_FLAG_IS_ARRAY, { SECTION_ID_PIXEL_FORMAT, -1 } },
        [SECTION_ID_PIXEL_FORMAT] =       { SECTION_ID_PIXEL_FORMAT, "pixel_format", 0, { SECTION_ID_PIXEL_FORMAT_FLAGS, SECTION_ID_PIXEL_FORMAT_COMPONENTS, -1 } },
        [SECTION_ID_PIXEL_FORMAT_FLAGS] = { SECTION_ID_PIXEL_FORMAT_FLAGS, "flags", 0, { -1 }, .unique_name = "pixel_format_flags" },
        [SECTION_ID_PIXEL_FORMAT_COMPONENTS] = { SECTION_ID_PIXEL_FORMAT_COMPONENTS, "components", SECTION_FLAG_IS_ARRAY, {SECTION_ID_PIXEL_FORMAT_COMPONENT, -1 }, .unique_name = "pixel_format_components" },
        [SECTION_ID_PIXEL_FORMAT_COMPONENT]  = { SECTION_ID_PIXEL_FORMAT_COMPONENT, "component", 0, { -1 } },
        [SECTION_ID_PROGRAM_STREAM_DISPOSITION] = { SECTION_ID_PROGRAM_STREAM_DISPOSITION, "disposition", 0, { -1 }, .unique_name = "program_stream_disposition" },
        [SECTION_ID_PROGRAM_STREAM_TAGS] =        { SECTION_ID_PROGRAM_STREAM_TAGS, "tags", SECTION_FLAG_HAS_VARIABLE_FIELDS, { -1 }, .element_name = "tag", .unique_name = "program_stream_tags" },
        [SECTION_ID_PROGRAM] =                    { SECTION_ID_PROGRAM, "program", 0, { SECTION_ID_PROGRAM_TAGS, SECTION_ID_PROGRAM_STREAMS, -1 } },
        [SECTION_ID_PROGRAM_STREAMS] =            { SECTION_ID_PROGRAM_STREAMS, "streams", SECTION_FLAG_IS_ARRAY, { SECTION_ID_PROGRAM_STREAM, -1 }, .unique_name = "program_streams" },
        [SECTION_ID_PROGRAM_STREAM] =             { SECTION_ID_PROGRAM_STREAM, "stream", 0, { SECTION_ID_PROGRAM_STREAM_DISPOSITION, SECTION_ID_PROGRAM_STREAM_TAGS, -1 }, .unique_name = "program_stream" },
        [SECTION_ID_PROGRAM_TAGS] =               { SECTION_ID_PROGRAM_TAGS, "tags", SECTION_FLAG_HAS_VARIABLE_FIELDS, { -1 }, .element_name = "tag", .unique_name = "program_tags" },
        [SECTION_ID_PROGRAM_VERSION] =    { SECTION_ID_PROGRAM_VERSION, "program_version", 0, { -1 } },
        [SECTION_ID_PROGRAMS] =                   { SECTION_ID_PROGRAMS, "programs", SECTION_FLAG_IS_ARRAY, { SECTION_ID_PROGRAM, -1 } },
        [SECTION_ID_ROOT] =               { SECTION_ID_ROOT, "root", SECTION_FLAG_IS_WRAPPER,
                                            { SECTION_ID_CHAPTERS, SECTION_ID_FORMAT, SECTION_ID_FRAMES, SECTION_ID_PROGRAMS, SECTION_ID_STREAMS,
                                              SECTION_ID_PACKETS, SECTION_ID_ERROR, SECTION_ID_PROGRAM_VERSION, SECTION_ID_LIBRARY_VERSIONS,
                                              SECTION_ID_PIXEL_FORMATS, -1} },
        [SECTION_ID_STREAMS] =            { SECTION_ID_STREAMS, "streams", SECTION_FLAG_IS_ARRAY, { SECTION_ID_STREAM, -1 } },
        [SECTION_ID_STREAM] =             { SECTION_ID_STREAM, "stream", 0, { SECTION_ID_STREAM_DISPOSITION, SECTION_ID_STREAM_TAGS, SECTION_ID_STREAM_SIDE_DATA_LIST, -1 } },
        [SECTION_ID_STREAM_DISPOSITION] = { SECTION_ID_STREAM_DISPOSITION, "disposition", 0, { -1 }, .unique_name = "stream_disposition" },
        [SECTION_ID_STREAM_TAGS] =        { SECTION_ID_STREAM_TAGS, "tags", SECTION_FLAG_HAS_VARIABLE_FIELDS, { -1 }, .element_name = "tag", .unique_name = "stream_tags" },
        [SECTION_ID_STREAM_SIDE_DATA_LIST] ={ SECTION_ID_STREAM_SIDE_DATA_LIST, "side_data_list", SECTION_FLAG_IS_ARRAY, { SECTION_ID_STREAM_SIDE_DATA, -1 }, .element_name = "side_data", .unique_name = "stream_side_data_list" },
        [SECTION_ID_STREAM_SIDE_DATA] =     { SECTION_ID_STREAM_SIDE_DATA, "side_data", 0, { -1 } },
        [SECTION_ID_SUBTITLE] =           { SECTION_ID_SUBTITLE, "subtitle", 0, { -1 } },
};

//OPTION
const char program_name[] = "ffprobe";
const int program_birth_year = 2007;

static int do_bitexact = 0;
static int do_count_frames = 0;
static int do_count_packets = 0;
static int do_read_frames  = 0;
static int do_read_packets = 0;
static int do_show_chapters = 0;
static int do_show_error   = 0;
static int do_show_format  = 0;
static int do_show_frames  = 0;
static int do_show_packets = 0;
static int do_show_programs = 0;
static int do_show_streams = 0;
static int do_show_stream_disposition = 0;
static int do_show_data    = 0;
static int do_show_program_version  = 0;
static int do_show_library_versions = 0;
static int do_show_pixel_formats = 0;
static int do_show_pixel_format_flags = 0;
static int do_show_pixel_format_components = 0;
static int do_show_log = 0;

static int do_show_chapter_tags = 0;
static int do_show_format_tags = 0;
static int do_show_frame_tags = 0;
static int do_show_program_tags = 0;
static int do_show_stream_tags = 0;
static int do_show_packet_tags = 0;

static int show_value_unit              = 0;
static int use_value_prefix             = 0;
static int use_byte_value_binary_prefix = 0;
static int use_value_sexagesimal_format = 0;
static int show_private_data            = 1;

static char *print_format;
static char *stream_specifier;
static char *show_data_hash;


static int opt_format(void *optctx, const char *opt, const char *arg);
static int opt_pretty(void *optctx, const char *opt, const char *arg);
static int opt_sections(void *optctx, const char *opt, const char *arg);
static int opt_show_format_entry(void *optctx, const char *opt, const char *arg);
static int opt_show_entries(void *optctx, const char *opt, const char *arg);
static int opt_show_versions(const char *opt, const char *arg);
static int opt_read_intervals(void *optctx, const char *opt, const char *arg);
static void opt_input_file(void *optctx, const char *arg);
static int opt_input_file_i(void *optctx, const char *opt, const char *arg);

static const OptionDef *options;

static const OptionDef real_options[] = {
#include "cmdutils_common_opts.h"
        { "f", HAS_ARG, {.func_arg = opt_format}, "force format", "format" },
        { "unit", OPT_BOOL, {&show_value_unit}, "show unit of the displayed values" },
        { "prefix", OPT_BOOL, {&use_value_prefix}, "use SI prefixes for the displayed values" },
        { "byte_binary_prefix", OPT_BOOL, {&use_byte_value_binary_prefix},
          "use binary prefixes for byte units" },
        { "sexagesimal", OPT_BOOL,  {&use_value_sexagesimal_format},
          "use sexagesimal format HOURS:MM:SS.MICROSECONDS for time units" },
        { "pretty", 0, {.func_arg = opt_pretty},
          "prettify the format of displayed values, make it more human readable" },
        { "print_format", OPT_STRING | HAS_ARG, {(void*)&print_format},
          "set the output printing format (available formats are: default, compact, csv, flat, ini, json, xml)", "format" },
        { "of", OPT_STRING | HAS_ARG, {(void*)&print_format}, "alias for -print_format", "format" },
        { "select_streams", OPT_STRING | HAS_ARG, {(void*)&stream_specifier}, "select the specified streams", "stream_specifier" },
        { "sections", OPT_EXIT, {.func_arg = opt_sections}, "print sections structure and section information, and exit" },
        { "show_data",    OPT_BOOL, {(void*)&do_show_data}, "show packets data" },
        { "show_data_hash", OPT_STRING | HAS_ARG, {(void*)&show_data_hash}, "show packets data hash" },
        { "show_error",   0, {(void*)&opt_show_error},  "show probing error" },
        { "show_format",  0, {(void*)&opt_show_format}, "show format/container info" },
        { "show_frames",  0, {(void*)&opt_show_frames}, "show frames info" },
        { "show_format_entry", HAS_ARG, {.func_arg = opt_show_format_entry},
          "show a particular entry from the format/container info", "entry" },
        { "show_entries", HAS_ARG, {.func_arg = opt_show_entries},
          "show a set of specified entries", "entry_list" },
#if HAVE_THREADS
        { "show_log", OPT_INT|HAS_ARG, {(void*)&do_show_log}, "show log" },
#endif
        { "show_packets", 0, {(void*)&opt_show_packets}, "show packets info" },
        { "show_programs", 0, {(void*)&opt_show_programs}, "show programs info" },
        { "show_streams", 0, {(void*)&opt_show_streams}, "show streams info" },
        { "show_chapters", 0, {(void*)&opt_show_chapters}, "show chapters info" },
        { "count_frames", OPT_BOOL, {(void*)&do_count_frames}, "count the number of frames per stream" },
        { "count_packets", OPT_BOOL, {(void*)&do_count_packets}, "count the number of packets per stream" },
        { "show_program_version",  0, {(void*)&opt_show_program_version},  "show ffprobe version" },
        { "show_library_versions", 0, {(void*)&opt_show_library_versions}, "show library versions" },
        { "show_versions",         0, {(void*)&opt_show_versions}, "show program and library versions" },
        { "show_pixel_formats", 0, {(void*)&opt_show_pixel_formats}, "show pixel format descriptions" },
        { "show_private_data", OPT_BOOL, {(void*)&show_private_data}, "show private data" },
        { "private",           OPT_BOOL, {(void*)&show_private_data}, "same as show_private_data" },
        { "bitexact", OPT_BOOL, {&do_bitexact}, "force bitexact output" },
        { "read_intervals", HAS_ARG, {.func_arg = opt_read_intervals}, "set read intervals", "read_intervals" },
        { "default", HAS_ARG | OPT_AUDIO | OPT_VIDEO | OPT_EXPERT, {.func_arg = opt_default}, "generic catch all option", "" },
        { "i", HAS_ARG, {.func_arg = opt_input_file_i}, "read specified file", "input_file"},
        { NULL, },
};

//READER
typedef struct ReadInterval {
    int id;             ///< identifier
    int64_t start, end; ///< start, end in second/AV_TIME_BASE units
    int has_start, has_end;
    int start_is_offset, end_is_offset;
    int duration_frames;
} ReadInterval;

static ReadInterval *read_intervals;
static int read_intervals_nb = 0;


//WRITER

/* WRITERS */

#define DEFINE_WRITER_CLASS(name)                   \
static const char *name##_get_name(void *ctx)       \
{                                                   \
    return #name ;                                  \
}                                                   \
static const AVClass name##_class = {               \
    .class_name = #name,                            \
    .item_name  = name##_get_name,                  \
    .option     = name##_options                    \
}

typedef struct Writer {
    const AVClass *priv_class;      ///< private class of the writer, if any
    int priv_size;                  ///< private size for the writer context
    const char *name;

    int  (*init)  (struct WriterContext *wctx);
    void (*uninit)(struct WriterContext *wctx);

    void (*print_section_header)(struct WriterContext *wctx);
    void (*print_section_footer)(struct WriterContext *wctx);
    void (*print_integer)       (struct WriterContext *wctx, const char *, long long int);
    void (*print_rational)      (struct WriterContext *wctx, AVRational *q, char *sep);
    void (*print_string)        (struct WriterContext *wctx, const char *, const char *);
    int flags;                  ///< a combination or WRITER_FLAG_*
} Writer;

#define SECTION_MAX_NB_LEVELS 10
#define PRINT_BUF_SIZE 4092

struct WriterContext {
    const AVClass *class;           ///< class of the writer
    const Writer *writer;           ///< the Writer of which this is an instance
    char *name;                     ///< name of this writer instance
    void *priv;                     ///< private data for use by the filter
    char buffer[PRINT_BUF_SIZE];

    const struct section *sections; ///< array containing all sections
    int nb_sections;                ///< number of sections

    int level;                      ///< current level, starting from 0

    /** number of the item printed in the given section, starting from 0 */
    unsigned int nb_item[SECTION_MAX_NB_LEVELS];

    /** section per each level */
    const struct section *section[SECTION_MAX_NB_LEVELS];
    AVBPrint section_pbuf[SECTION_MAX_NB_LEVELS]; ///< generic print buffer dedicated to each section,
    ///  used by various writers

    unsigned int nb_section_packet; ///< number of the packet section in case we are in "packets_and_frames" section
    unsigned int nb_section_frame;  ///< number of the frame  section in case we are in "packets_and_frames" section
    unsigned int nb_section_packet_frame; ///< nb_section_packet or nb_section_frame according if is_packets_and_frames

    int string_validation;
    char *string_validation_replacement;
    unsigned int string_validation_utf8_flags;
};

typedef struct WriterContext WriterContext;

#define WRITER_FLAG_DISPLAY_OPTIONAL_FIELDS 1
#define WRITER_FLAG_PUT_PACKETS_AND_FRAMES_IN_SAME_CHAPTER 2

//XML
typedef struct XMLContext {
    const AVClass *class;
    int within_tag;
    int indent_level;
    int fully_qualified;
    int xsd_strict;
} XMLContext;

static av_cold int xml_init(struct WriterContext *wctx);
static void xml_print_section_header(struct WriterContext *wctx);
static void xml_print_section_footer(struct WriterContext *wctx);
static void xml_print_str(struct WriterContext *wctx, const char *key, const char *value);
static void xml_print_int(struct WriterContext *wctx, const char *key, long long int value);

#undef OFFSET
#define OFFSET(x) offsetof(XMLContext, x)

static const AVOption xml_options[] = {
        {"fully_qualified", "specify if the output should be fully qualified", OFFSET(fully_qualified), AV_OPT_TYPE_BOOL, {.i64=0}, 0, 1},
        {"q",               "specify if the output should be fully qualified", OFFSET(fully_qualified), AV_OPT_TYPE_BOOL, {.i64=0}, 0, 1},
        {"xsd_strict",      "ensure that the output is XSD compliant",         OFFSET(xsd_strict),      AV_OPT_TYPE_BOOL, {.i64=0}, 0, 1},
        {"x",               "ensure that the output is XSD compliant",         OFFSET(xsd_strict),      AV_OPT_TYPE_BOOL, {.i64=0}, 0, 1},
        {NULL},
};

DEFINE_WRITER_CLASS(xml);

static Writer xml_writer = {
        .name                 = "xml",
        .priv_size            = sizeof(XMLContext),
        .init                 = xml_init,
        .print_section_header = xml_print_section_header,
        .print_section_footer = xml_print_section_footer,
        .print_integer        = xml_print_int,
        .print_string         = xml_print_str,
        .flags = WRITER_FLAG_PUT_PACKETS_AND_FRAMES_IN_SAME_CHAPTER,
        .priv_class           = &xml_class,
};


//probe
static void ffprobe_cleanup(int ret);

#define SET_DO_SHOW(id, varname) do {                                   \
        if (check_section_show_entries(SECTION_ID_##id))                \
            do_show_##varname = 1;                                      \
    } while (0)

static inline int check_section_show_entries(int section_id);
static void writer_register_all(void);
static const Writer *writer_get_by_name(const char *name);
static int writer_open(struct WriterContext **wctx, const Writer *writer, const char *args,
                       const struct section *sections, int nb_sections);
static inline void writer_print_section_header(WriterContext *wctx, int section_id);
static inline void writer_print_section_footer(WriterContext *wctx);
static void ffprobe_show_program_version(WriterContext *w);
static void ffprobe_show_library_versions(WriterContext *w);
static void ffprobe_show_pixel_formats(WriterContext *w);
static void show_usage(void);
static int probe_file(WriterContext *wctx, const char *filename);
static void show_error(WriterContext *w, int err);
static void writer_close(WriterContext **wctx);

//out
static char* probe(int argc, char **argv);


#endif //FFPROBE_FFPROBE_H
