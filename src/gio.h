/**************************************************************************/
// gio.h - Generic IO header to include to use the gio system
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#ifndef GIO_H
#define GIO_H

#ifndef END
// end of record
#define END_RECORD "END"
#endif
// end of file
#ifndef EOF
#define END_FILE "EOF~"
#endif

#define GIOFLAG_ALWAYS_WRITE	(A)

/**************************************************************************/
enum saveTypes { END, STR, _INT, _LONG, SHINT, _CHAR, _BOOL, WFLAG, SHWFLAG,
				STR_ARRAY, STR_ARRAYLIST, INT_ARRAY, LONG_ARRAY, 
				SHINT_ARRAY, BOOL_ARRAY,
				CUSTOM_READ,
				CUSTOM_WRITE,
				READ_TO_EOL, // used for ignore old single line fields
				READ_TO_END_OF_STRING, // used for ignore and old string entry
				CUSTOM_DONT_SAVE_RECORD, // a custom function used to ignore saving records
				VN_INT };

// END - marks the end of a table and also stores the size of the 
// structure in bytes as its index value
/**************************************************************************/
struct gio_type{
	int index; 
	const char *heading;
	saveTypes type;
	int flags;
	union {
		void *pvoid; // extra info, wordflag = table, bool = bit number
		flag_type *pflag_type;
		short sval;
		int	ival;
		long lval;		
	};
};
/**************************************************************************/
// prototypes
FILE *saveRecord(gio_type *gio_table, void *data, FILE *fp, int *status);
int   loadRecord(gio_type *gio_table, void *data, FILE *fp);

// custom function type
typedef void GIO_CUSTOM_FUNCTION (gio_type *gioTable, 
		int tableIndex, void *data, FILE *fp);

#define GIO_CUSTOM_FUNCTION_PROTOTYPE( funcname)	\
						void funcname(gio_type *, int , void *, FILE *)

// custom return function type - returns a status code
typedef int (*GIO_CUSTOM_RETURN_FUNCTION) (gio_type *gioTable, 
		int tableIndex, void *data, FILE *fp);

#define GIO_CUSTOM_RETURN_FUNCTION_PROTOTYPE( funcname)	\
						int funcname(gio_type *, int , void *, FILE *)

/**************************************************************************/
// ####### GENERIC IO MACROS ####### 

// start the generic IO table
// - this macro creates the header of a function that returns a gio_type pointer
//   to a static table embedded within the function.
//   the function name is based on the name of the datatype you tell it you are 
//   using... ie GIO_START(race_type_old) would generate a function which prototype 
//   would be  gio_type * gio_tlookup_race_type_old();
#define GIO_START(t) gio_type * gio_tlookup_ ## t (){ t gio_this_table_type; \
	int GIO_structSize=(int)sizeof(t); static gio_type gio_table[]= { 

#define GIO_PROTOTYPE(t) gio_type * gio_tlookup_ ## t ();

// GIO_GET_INDEX is used to find out how many bytes from the start of a 
//   structure the requested field is stored.
#define GIO_GET_INDEX(field) (int)(((char*) &gio_this_table_type.field) \
		-((char*) &gio_this_table_type)) 
// GIO_STRH(field, header) is used for string fields with a custom header
//  - basically adds an entry into the table bedded within the 
//    gio_type * gio_tlookup_?????(); function that would contain 
//    { index_of_the_field, header as a string , STR, 0 },
// You need to put ""'s around the header
#define GIO_STRH(field,header) { GIO_GET_INDEX(field), header, STR, 0, {NULL}},
#define GIO_STRH_FLAGS(field,header, flags) \
		{ GIO_GET_INDEX(field), header,  STR, flags, {NULL} },
// GIO_STR(field) is used for string fields with no custom header
#define GIO_STR(field) { GIO_GET_INDEX(field), #field, STR, 0, {(void *)NULL} },
#define GIO_STR_FLAGS(field, flags) \
		{ GIO_GET_INDEX(field), #field, STR, flags, {(void *)NULL} },

// string array
#define GIO_STR_ARRAY(field, max) { GIO_GET_INDEX(field), #field, STR_ARRAY, 0, {(void *)max}},
// string array with header
#define GIO_STR_ARRAYH(field, header, max) { GIO_GET_INDEX(field), header, STR_ARRAY, 0, {(void *)max}},

// string array list - when you reach the first null string, stop
#define GIO_STR_ARRAYLIST(field, max) \
	{ GIO_GET_INDEX(field), #field, STR_ARRAYLIST, 0, {(void *)max}},
// string array with header
#define GIO_STR_ARRAYLISTH(field, header, max) \
	{ GIO_GET_INDEX(field), header, STR_ARRAYLIST, 0, {(void *)max}},


// GIO_INTH(field, header) is used for integer fields with a custom header
#define GIO_INTH(field,header) { GIO_GET_INDEX(field), header, _INT, 0, {NULL}},
#define GIO_INTH_FLAGS(field,header, flags) \
		{ GIO_GET_INDEX(field), header, _INT, flags, {NULL}},
// GIO_INT(field) is used for integer fields with no custom header
#define GIO_INT(field) { GIO_GET_INDEX(field), #field, _INT, 0, {NULL}},
#define GIO_INT_FLAGS(field, flags) \
		{ GIO_GET_INDEX(field), #field, _INT, flags, {NULL}},

// GIO_INT_WITH_DEFAULT(field) is used for integer fields with no custom header
// that has a default value for loading, if it reads in 0, the value is set to this
#define GIO_INT_WITH_DEFAULT(field, default_value) \
	{ GIO_GET_INDEX(field), #field, _INT, 0, {(void *)default_value}},
#define GIO_INT_WITH_DEFAULT_FLAGS(field, default_value, flags) \
	{ GIO_GET_INDEX(field), #field, _INT, flags, {(void *)default_value}},

// GIO_LONGH(field, header) is used for long fields with a custom header
#define GIO_LONGH(field,header) { GIO_GET_INDEX(field), header, _LONG, 0, {NULL}},
#define GIO_LONGH_FLAGS(field,header, flags) \
		{ GIO_GET_INDEX(field), header, _LONG, flags, {NULL}},
// GIO_LONG(field) is used for long fields with no custom header
#define GIO_LONG(field) { GIO_GET_INDEX(field), #field, _LONG, 0, {NULL}},
#define GIO_LONG_FLAGS(field, flags) \
		{ GIO_GET_INDEX(field), #field, _LONG, flags, {NULL}},

// int array 
#define GIO_INT_ARRAY(field, max) { GIO_GET_INDEX(field), #field, INT_ARRAY, 0, {(void *)max}},
// int array with header
#define GIO_INT_ARRAYH(field, header, max) { GIO_GET_INDEX(field), header, INT_ARRAY, 0, {(void *)max}},

// long array 
#define GIO_LONG_ARRAY(field, max) { GIO_GET_INDEX(field), #field, LONG_ARRAY, 0, {(void *)max}},
// long array with header
#define GIO_LONG_ARRAYH(field, header, max) { GIO_GET_INDEX(field), header, LONG_ARRAY, 0, {(void *)max}},

// GIO_SHINTH(field, header) is used for short ints 
#define GIO_SHINTH(field,header) { GIO_GET_INDEX(field), header, SHINT, 0, {NULL}},
// GIO_SHINT(field) is used for short ints 
#define GIO_SHINT(field) { GIO_GET_INDEX(field), #field, SHINT, 0, {NULL}},

// GIO_SHINT_WITH_DEFAULT(field) is used for integer fields with no custom header
// that has a default value for loading, if it reads in 0, the value is set to this
#define GIO_SHINT_WITH_DEFAULT(field, default_value) \
	{ GIO_GET_INDEX(field), #field, SHINT, 0, {(void *)default_value}},

// shint array 
#define GIO_SHINT_ARRAY(field, max) { GIO_GET_INDEX(field), #field, SHINT_ARRAY, 0, {(void *)max}},
// shint array with header
#define GIO_SHINT_ARRAYH(field, header, max) { GIO_GET_INDEX(field), header, SHINT_ARRAY, 0, {(void *)max}},

// GIO_CHAR(field, header) is used for fields of type char
#define GIO_CHARH(field,header) { GIO_GET_INDEX(field), header, _CHAR, 0, {NULL}},
// GIO_CHAR(field) is used for fields of type char
#define GIO_CHAR(field) { GIO_GET_INDEX(field), #field, _CHAR, 0, {NULL}},

// GIO_BOOLH(field, header) is used for boolean fields with a custom header
// it will actually write a TRUE/FALSE in the file
#define GIO_BOOLH(field,header) { GIO_GET_INDEX(field), header, _BOOL, 0, {(void*)~0}},
// GIO_BOOL(field) is used for boolean fields with no custom header
#define GIO_BOOL(field) { GIO_GET_INDEX(field), #field, _BOOL, 0, {(void*)~0}},
// header and specify the bitmask to check on a long
#define GIO_BOOLHM(field,header,mask) { GIO_GET_INDEX(field), header, _BOOL, 0, {(void *)mask}},
// specify the bitmask to check on a long
#define GIO_BOOLM(field, mask) { GIO_GET_INDEX(field), #field, _BOOL, 0, {(void *)mask}},

// GIO_WFLAGH(field, header) is used for wordflag fields with a custom header
#define GIO_WFLAGH(field,header, table) { GIO_GET_INDEX(field), header, WFLAG, 0, {(void *)&table}},
// GIO_WFLAG(field) is used for wordflag fields with no custom header
#define GIO_WFLAG(field, table) { GIO_GET_INDEX(field), #field, WFLAG, 0, {(void *)&table}},

// GIO_SHWFLAGH(field, header) is used for wordflag fields with a custom header (sh_int input)
#define GIO_SHWFLAGH(field,header, table) { GIO_GET_INDEX(field), header, SHWFLAG, 0, {(void *)&table}},
// GIO_SHWFLAG(field) is used for wordflag fields with no custom header (sh_int input)
#define GIO_SHWFLAG(field, table) { GIO_GET_INDEX(field), #field, SHWFLAG, 0, {(void *)&table}},

// GIO_CUSTOM_READ allows you to specify a function to run for reading when the 
// header is encountered - the function will be given the GIO table index and the 
// fp as parameters.
#define GIO_CUSTOM_READ(field,function) \
	{ GIO_GET_INDEX(field), #field, CUSTOM_READ, 0, {(void *) function}},
#define GIO_CUSTOM_READH(field, header, function) \
		{ GIO_GET_INDEX(field), header, CUSTOM_READ, 0, {(void *) function}},

// GIO_CUSTOM_WRITE allows you to specify a function to run for reading when the 
// header is encountered - the function will be given the GIO table index and the 
// fp as parameters.
#define GIO_CUSTOM_WRITE(field,function) \
	{ GIO_GET_INDEX(field), #field, CUSTOM_WRITE, 0, {(void *) function}},
#define GIO_CUSTOM_WRITEH(field, header, function) \
		{ GIO_GET_INDEX(field), header, CUSTOM_WRITE, 0, {(void *) function}},


// GIO_READ_TO_EOL allows you to remove stuff and put in this entry
// to aid in the conversion process.
#define GIO_READ_TO_EOL(header) { 0, header, READ_TO_EOL, 0, {NULL}},

// GIO_READ_TO_END_OF_STRING allows you to remove stuff and put in 
// this entry to aid in the conversion process.
#define GIO_READ_TO_END_OF_STRING(header) { 0, header, READ_TO_END_OF_STRING, 0, {NULL}},

// GIO_CUSTOM_DONT_SAVE_RECORD allows you to specify a custom function to be run 
// just prior to deciding to write the data for a particular record into a GIO
// saving list. GIO_CUSTOM_DONT_SAVE_RECORD should be the first in the table.
// - the function will be given the GIO table index and the fp as parameters
//   and needs to return true to indicate the entry is to not be saved.
#define GIO_CUSTOM_DONT_SAVE_RECORD(function) \
	{ 0, "dontsavefunctionpointer", READ_TO_END_OF_STRING, 0, {(void *) function}},

// finishes off the generic IO table, then tells the function to return 
// a pointer to the embedded static table, after which it closes the function.
#define GIO_FINISH { GIO_structSize, "", END, 0, {NULL}} }; return gio_table; };

// NOCLEAR means the table entry isn't cleared when it is created
#define GIO_FINISH_NOCLEAR { GIO_structSize, "gio-noclear", END, 0, {NULL}} }; return gio_table; };

// GIO_FINISH_STRDUP_EMPTY  means strings get a str_dup("")
#define GIO_FINISH_STRDUP_EMPTY { GIO_structSize, "strdup_empty", END, 0, {NULL}} }; return gio_table; };

#define GIO_SAVE_RECORD(datatype, data, fp, status) \
	saveRecord( gio_tlookup_ ## datatype (), data, fp, status) 

#define GIO_LOAD_RECORD(datatype, data, fp) \
	loadRecord( gio_tlookup_ ## datatype (), data, fp) 

/**************************************************************************/
// gio_generic_savelist returns true if no errors on save
bool	gio_generic_savelist( void *listpointer, gio_type *gio_table, \
				const char *filename, int nextoffset, bool backup);
void * gio_generic_loadlist( int size, gio_type *gio_table, \
							const char *filename, int nextoffset);

#define GIO_GET_NEXTOFFSET(listpointer) (long)( ((char*)&(listpointer->next)) \
		-((char*)listpointer) )

#define GIOSAVE_LIST(listpointer, datatype, filename, backup) \
	gio_generic_savelist( listpointer, gio_tlookup_ ## datatype (), \
							filename, GIO_GET_NEXTOFFSET(listpointer), backup);

#define GIOLOAD_LIST(listpointer, datatype, filename) \
	listpointer=(datatype*)gio_generic_loadlist( sizeof(datatype), \
		gio_tlookup_ ## datatype (),  filename, \
		GIO_GET_NEXTOFFSET(listpointer));

int		wordflag_to_value( const struct flag_type *flag_table, char *wf_str);

#endif // GIO
