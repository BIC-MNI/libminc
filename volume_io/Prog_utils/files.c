
#include  <def_mni.h>
#include  <def_files.h>
#include  <def_alloc.h>
#include  <def_string.h>

/* ----------------------------- MNI Header -----------------------------------
@NAME       : file_exists
@INPUT      : filename
@OUTPUT     : 
@RETURNS    : TRUE or FALSE if file exists
@DESCRIPTION: Checks if the file of the given name exists
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  Boolean  file_exists( filename )
    char        filename[];
{
    Boolean  exists;
    FILE     *file;

    file = fopen( filename, "r" );

    if( file != (FILE *) 0 )
    {
        (void) fclose( file );
        exists = TRUE;
    }
    else
        exists = FALSE;

    return( exists );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : remove_file
@INPUT      : filename
@OUTPUT     : 
@RETURNS    : 
@DESCRIPTION: Deletes the given file.
@METHOD     : Makes a system call to perform a UNIX "rm"
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  void  remove_file( filename )
    char  filename[];
{
    String  command;

    (void) sprintf( command, "rm -f %s", filename );

    (void) system( command );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : open_file
@INPUT      : filename
            : io_type        READ_FILE or WRITE_FILE
            : file_format    ASCII_FORMAT or BINARY_FORMAT
@OUTPUT     : file
@RETURNS    : 
@DESCRIPTION: Opens the given filename for ascii or binary input or output.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  Status  open_file( filename, io_type, file_format, file )
    char               filename[];
    IO_types           io_type;
    File_formats       file_format;
    FILE               **file;
{
    Status   status;
    String   access_str;

    switch( io_type )
    {
    case APPEND_FILE:   (void) strcpy( access_str, "a" );  break;

    case WRITE_FILE:    (void) strcpy( access_str, "w" );  break;

    case READ_FILE:
    default:            (void) strcpy( access_str, "r" );  break;
    }

    if( file_format == BINARY_FORMAT )
        (void) strcat( access_str, "b" );

    *file = fopen( filename, access_str );

    if( *file != (FILE *) 0 )
    {
        status = OK;
    }
    else
    {
        PRINT_ERROR( "Could not open file \"" );
        PRINT_ERROR( filename );
        PRINT_ERROR( "\".\n" );
        *file = (FILE *) 0;
        status = ERROR;
    }

    return( status );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : open_file_with_default_suffix
@INPUT      : filename
            : default_suffix  - e.g. ".obj"
            : io_type        READ_FILE or WRITE_FILE
            : file_format    ASCII_FORMAT or BINARY_FORMAT
@OUTPUT     : file
@RETURNS    : 
@DESCRIPTION: Opens the given filename for ascii or binary input or output.
            : On output, if the file has no suffix, it adds the default suffix.
            : On input, if the file does not exist as given, then it tries to
            : find the file with the default_suffix.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  Status  open_file_with_default_suffix( filename, default_suffix,
                                               io_type, file_format, file )
    char               filename[];
    char               default_suffix[];
    IO_types           io_type;
    File_formats       file_format;
    FILE               **file;
{
    Boolean  suffix_added;
    String   used_filename;

    if( io_type == READ_FILE )
    {
        suffix_added = FALSE;

        if( !file_exists(filename) && has_no_extension( filename ) )
        {
            (void) sprintf( used_filename, "%s.%s", filename, default_suffix );
            if( file_exists( used_filename ) )
                suffix_added = TRUE;
        }

        if( !suffix_added )
            (void) strcpy( used_filename, filename );
    }
    else if( has_no_extension( filename ) )
        (void) sprintf( used_filename, "%s.%s", filename, default_suffix );
    else
        (void) strcpy( used_filename, filename );

    return( open_file( used_filename, io_type, file_format, file ) );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : has_no_extension
@INPUT      : filename
@OUTPUT     : 
@RETURNS    : TRUE if there is no . extension
@DESCRIPTION: Checks if there is an extension on the file.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

private  Boolean  has_no_extension( filename )
    char   filename[];
{
    char  *str;

    /* skip possible .. at beginning */

    str = filename;
    while( *str == '.' ) ++str;

    return( strchr( str, '.' ) == (char *) 0 );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : set_file_position
@INPUT      : file
            : byte_position
@OUTPUT     : 
@RETURNS    : 
@DESCRIPTION: Sets the file position to the given offset from the start.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  Status  set_file_position( file, byte_position )
    FILE     *file;
    long     byte_position;
{
    Status   status;

    if( fseek( file, byte_position, 0 ) == 0 )
    {
        status = OK;
    }
    else
    {
        PRINT_ERROR( "Error setting the file position.\n" );
        status = ERROR;
    }

    return( status );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : close_file
@INPUT      : file
@OUTPUT     : 
@RETURNS    : 
@DESCRIPTION: Closes the file.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  Status  close_file( file )
    FILE     *file;     
{
    (void) fclose( file );

    return( OK );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : extract_directory
@INPUT      : filename
@OUTPUT     : directory
@RETURNS    : 
@DESCRIPTION: Extracts the directory from the filename by copying the string
            : from the beginning up to the last '/'.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  void  extract_directory( filename, directory )
    char    filename[];
    char    directory[];
{
    int   slash_index;

    slash_index = strlen(filename) - 1;

    while( slash_index > 0 && filename[slash_index] != '/' )
    {
        --slash_index;
    }

    if( slash_index < 0 )
    {
        slash_index = 0;
    }

    (void) strncpy( directory, filename, slash_index );

    directory[slash_index] = (char) 0;
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : get_absolute_filename
@INPUT      : filename
            : directory
@OUTPUT     : abs_filename
@RETURNS    : 
@DESCRIPTION: Given a filename and a default directory, determines the correct
            : filename by checking if the filename is a relative or absolute
            : pathname, and prepending the directory, if the former.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  void  get_absolute_filename( filename, directory, abs_filename )
    char    filename[];
    char    directory[];
    char    abs_filename[];
{
    String  save_filename;

    /* in case abs_filename and filename are same variable */

    (void) strcpy( save_filename, filename );

    /* if the directory is non-null and the filename is not already
       absolute (begins with '/'), then prefix the directory to the filename */

    if( directory != (char *) 0 && strlen( directory ) > 0 &&
        filename[0] != '/' )
    {
        (void) strcpy( abs_filename, directory );
        (void) strcat( abs_filename, "/" );
    }
    else
    {
        abs_filename[0] = (char) 0;
    }

    (void) strcat( abs_filename, save_filename );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : strip_off_directories
@INPUT      : filename
@OUTPUT     : no_dirs
@RETURNS    : 
@DESCRIPTION: Strips off the directories from filename putting the result in
            : no_dirs.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  void  strip_off_directories( filename, no_dirs )
    char    filename[];
    char    no_dirs[];
{
    int    i;

    i = strlen( filename ) - 1;

    while( i >= 0 && filename[i] != '/' )
    {
        --i;
    }

    (void) strcpy( no_dirs, &filename[i+1] );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : flush_file
@INPUT      : file
@OUTPUT     : 
@RETURNS    : 
@DESCRIPTION: Flushes the output buffer for the given file.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  Status  flush_file( file )
    FILE     *file;
{
    Status   status;

    if( fflush( file ) == 0 )
    {
        status = OK;
    }
    else
    {
        PRINT_ERROR( "Error flushing file.\n" );
        status = ERROR;
    }

    return( status );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : input_character
@INPUT      : file
@OUTPUT     : ch
@RETURNS    : Status
@DESCRIPTION: Inputs one character from the file, returning ERROR if eof.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  Status  input_character( file, ch )
    FILE  *file;
    char   *ch;
{
    Status   status;
    int      c;

    c = fgetc( file );

    if( c == EOF )
    {
        status = ERROR;
    }
    else
    {
        *ch = c;
        status = OK;
    }

    return( status );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : unget_character
@INPUT      : file
@OUTPUT     : ch
@RETURNS    : Status
@DESCRIPTION: Ungets one character back to the file, returning status.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  Status  unget_character( file, ch )
    FILE  *file;
    char  ch;
{
    Status   status;
    int      c;

    c = ungetc( ch, file );

    if( c == EOF )
        status = ERROR;
    else
        status = OK;

    return( status );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : input_nonwhite_character
@INPUT      : file
@OUTPUT     : ch
@RETURNS    : Status
@DESCRIPTION: Inputs the next nonwhite (tab, space, newline) character.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  Status  input_nonwhite_character( file, ch )
    FILE   *file;
    char   *ch;
{
    Status   status;

    do
    {
        status = input_character( file, ch );
    }
    while( status == OK && (*ch == ' ' || *ch == '\t' || *ch == '\n') );

    return( status );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : output_character
@INPUT      : file
            : ch
@OUTPUT     : 
@RETURNS    : Status
@DESCRIPTION: Outputs the character to the file, returning the status.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  Status  output_character( file, ch )
    FILE   *file;
    char   ch;
{
    Status   status;

    if( fputc( ch, file ) != ch )
    {
        status = ERROR;
    }
    else
    {
        status = OK;
    }

    return( status );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : skip_input_until
@INPUT      : file
            : search_char
@OUTPUT     : 
@RETURNS    : Status
@DESCRIPTION: Skips characters in the file, up to and including the first match
            : of the search_char;
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  Status   skip_input_until( file, search_char )
    FILE   *file;
    char   search_char;
{
    Status   status;
    char     ch;

    status = OK;

    do
    {
        status = input_character( file, &ch );
    }
    while( status == OK && ch != search_char );

    return( status );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : output_string
@INPUT      : file
            : str
@OUTPUT     : 
@RETURNS    : Status
@DESCRIPTION: Outputs the string to the file.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  Status  output_string( file, str )
    FILE  *file;
    char  str[];
{
    Status   status;

    if( fprintf( file, "%s", str ) > 0 )
        status = OK;
    else
    {
        PRINT_ERROR( "Error outputting string.\n" );
        status = ERROR;
    }

    return( status );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : input_string
@INPUT      : file
            : str
            : string_length         - size of string storage
            : termination_char
@OUTPUT     : 
@RETURNS    : Status
@DESCRIPTION: Inputs a string from the file.  First it skips white space, then
            : inputs all characters until the termination_char is found.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  Status  input_string( file, str, string_length, termination_char )
    FILE  *file;
    char  str[];
    int   string_length;
    char  termination_char;
{
    int     i;
    char    ch;
    Status  status;

    status = input_nonwhite_character( file, &ch );

    i = 0;

    while( status == OK && ch != termination_char && ch != '\n' )
    {
        str[i] = ch;
        ++i;

        if( i >= string_length - 1 )
        {
            PRINT_ERROR( "Input string too long.\n" );
            status = ERROR;
        }
        else
        {
            status = input_character( file, &ch );
        }
    }

    if( ch == '\n' )
        (void) ungetc( ch, file );

    str[i] = (char) 0;
    
    return( status );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : input_quoted_string
@INPUT      : file
            : str
            : str_length    - size of string storage
@OUTPUT     : 
@RETURNS    : Status
@DESCRIPTION: Skips to the next nonwhitespace character, checks if it is a
            : quotation mark, then reads characters into the string until the
            : next quotation mark.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  Status  input_quoted_string( file, str, str_length )
    FILE            *file;
    char            str[];
    int             str_length;
{
    int      i;
    char     ch;
    Status   status;
    Status   input_nonwhite_character();
    Status   input_character();

    status = input_nonwhite_character( file, &ch );

    if( status == OK && ch != '"' )
        status = ERROR;

    if( status == OK )
        status = input_character( file, &ch );

    i = 0;

    while( status == OK && ch != '"' && i < str_length-1 )
    {
        str[i] = ch;
        ++i;
        status = input_character( file, &ch );
    }

    if( status == OK && ch == '\n' )
        (void) ungetc( ch, file );

    str[i] = (char) 0;

    return( status );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : output_quoted_string
@INPUT      : file
            : str
@OUTPUT     : 
@RETURNS    : Status
@DESCRIPTION: Outputs the given string, with quotation marks around it.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  Status  output_quoted_string( file, str )
    FILE            *file;
    char            str[];
{
    Status   status;

    if( fprintf( file, " \"%s\"", str ) > 0 )
        status = OK;
    else
        status = ERROR;

    return( status );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : input_binary_data
@INPUT      : file
            : element_size       size of each element
            : n                  number of elements
@OUTPUT     : data               array of elements to input
@RETURNS    : Status
@DESCRIPTION: Inputs the data in binary format.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  Status  input_binary_data( file, data, element_size, n )
    FILE            *file;
    VOID            *data;
    size_t          element_size;
    int             n;
{
    Status   status;
    int      n_done;

    status = OK;

    n_done = fread( data, element_size, n, file );
    if( n_done != n )
    {
#ifdef ERROR_MESSAGES
        PRINT_ERROR( "Error inputting binary data.\n" );
        PRINT( "     (%d out of %d items of size %d).\n", n_done, n,
               (int) element_size );
#endif
        status = ERROR;
    }

    return( status );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : output_binary_data
@INPUT      : file
            : data               array of elements to output
            : element_size       size of each element
            : n                  number of elements
@OUTPUT     : 
@RETURNS    : Status
@DESCRIPTION: Outputs the data in binary format.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  Status  output_binary_data( file, data, element_size, n )
    FILE            *file;
    VOID            *data;
    size_t          element_size;
    int             n;
{
    Status   status;
    int      n_done;

    status = OK;

    n_done = fwrite( data, element_size, n, file );
    if( n_done != n )
    {
        PRINT_ERROR( "Error outputting binary data.\n" );
        PRINT( "     (%d out of %d items of size %d).\n", n_done, n,
               (int) element_size );
        status = ERROR;
    }

    return( status );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : input_newline
@INPUT      : file
@OUTPUT     : 
@RETURNS    : Status
@DESCRIPTION: Skips to after the next newline in the file.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  Status  input_newline( file )
    FILE            *file;
{
    Status   status;
    Status   skip_input_until();

    status = skip_input_until( file, '\n' );

    if( status == END_OF_FILE )
    {
        PRINT_ERROR( "Error inputting newline.\n" );
        status = ERROR;
    }

    return( status );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : output_newline
@INPUT      : file
@OUTPUT     : 
@RETURNS    : Status
@DESCRIPTION: Outputs a newline to the file.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  Status  output_newline( file )
    FILE            *file;
{
    Status   status;

    if( fprintf( file, "\n" ) > 0 )
        status = OK;
    else
    {
        PRINT_ERROR( "Error outputting newline.\n" );
        status = ERROR;
    }

    return( status );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : input_line
@INPUT      : line         - string to input to
            : str_length   - storage allocated to the string
@OUTPUT     : 
@RETURNS    : Status
@DESCRIPTION: Inputs all characters upto the next newline.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  Status  input_line( file, line, str_length )
    FILE    *file;
    char    line[];
    int     str_length;
{
    Status   status;
    int      i;
    char     ch;

    i = 0;

    status = input_character( file, &ch );

    while( status == OK && ch != '\n' && i < str_length-1 )
    {
        line[i] = ch;
        ++i;

        status = input_character( file, &ch );
    }

    line[i] = (char) 0;

    return( status );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : input_boolean
@INPUT      : file
@OUTPUT     : b
@RETURNS    : Status
@DESCRIPTION: Inputs a Boolean value from a file, by looking for an 'f' or 't'.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  Status  input_boolean( file, b )
    FILE            *file;
    Boolean         *b;
{
    Status   status;
    char     ch;
    Status   input_nonwhite_character();

    status = input_nonwhite_character( file, &ch );

    if( status == OK )
    {
        if( ch == 'f' || ch == 'F' )
            *b = FALSE;
        else if( ch == 't' || ch == 'T' )
            *b = TRUE;
        else
            status = ERROR;
    }

    return( status );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : output_boolean
@INPUT      : file
            : b
@OUTPUT     : 
@RETURNS    : Status
@DESCRIPTION: Outputs a T or F to the file.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  Status  output_boolean( file, b )
    FILE            *file;
    Boolean         b;
{
    Status   status;
    char     *str;

    status = OK;

    if( b )
        str = "T";
    else
        str = "F";

    if( fprintf( file, " %s", str ) > 0 )
    {
        PRINT_ERROR( "Error outputting Boolean.\n" );
        status = ERROR;
    }

    return( status );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : input_short
@INPUT      : file
@OUTPUT     : s
@RETURNS    : Status
@DESCRIPTION: Inputs an ascii short.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  Status  input_short( file, s )
    FILE            *file;
    short           *s;
{
    Status   status;

    if( fscanf( file, "%hd", s ) == 1 )
        status = OK;
    else
        status = ERROR;

    return( status );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : output_short
@INPUT      : file
            : s
@OUTPUT     :
@RETURNS    : Status
@DESCRIPTION: Outputs an ascii short.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  Status  output_short( file, s )
    FILE            *file;
    short           s;
{
    Status   status;

    if( fprintf( file, " %d", s ) > 0 )
        status = OK;
    else
    {
        PRINT_ERROR( "Error outputting short.\n" );
        status = ERROR;
    }

    return( status );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : input_int
@INPUT      : file
@OUTPUT     : i
@RETURNS    : Status
@DESCRIPTION: Inputs an ascii integer.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  Status  input_int( file, i )
    FILE            *file;
    int             *i;
{
    Status   status;

    if( fscanf( file, "%d", i ) == 1 )
        status = OK;
    else
        status = ERROR;

    return( status );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : output_int
@INPUT      : file
            : i
@OUTPUT     :
@RETURNS    : Status
@DESCRIPTION: Outputs an ascii integer.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  Status  output_int( file, i )
    FILE            *file;
    int             i;
{
    Status   status;

    if( fprintf( file, " %d", i ) > 0 )
        status = OK;
    else
    {
        PRINT_ERROR( "Error outputting int.\n" );
        status = ERROR;
    }

    return( status );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : input_real
@INPUT      : file
@OUTPUT     : r
@RETURNS    : Status
@DESCRIPTION: Inputs an ascii real value.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  Status  input_real( file, r )
    FILE            *file;
    Real            *r;
{
    Status   status;

    if( fscanf( file, "%f", r ) == 1 )
        status = OK;
    else
    {
#ifdef ERROR_MESSAGES
        PRINT_ERROR( "Error inputting float.\n" );
#endif
        status = ERROR;
    }

    return( status );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : output_real
@INPUT      : file
            : i
@OUTPUT     :
@RETURNS    : Status
@DESCRIPTION: Outputs an ascii real value.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  Status  output_real( file, r )
    FILE            *file;
    Real            r;
{
    Status   status;

    if( fprintf( file, " %g", r ) > 0 )
        status = OK;
    else
    {
        PRINT_ERROR( "Error outputting float.\n" );
        status = ERROR;
    }

    return( status );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : input_double
@INPUT      : file
@OUTPUT     : d
@RETURNS    : Status
@DESCRIPTION: Inputs an ascii double.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  Status  input_double( file, d )
    FILE            *file;
    double          *d;
{
    Status   status;

    if( fscanf( file, "%lf", d ) == 1 )
        status = OK;
    else
    {
#ifdef ERROR_MESSAGES
        PRINT_ERROR( "Error inputting double.\n" );
#endif
        status = ERROR;
    }

    return( status );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : output_double
@INPUT      : file
            : d
@OUTPUT     :
@RETURNS    : Status
@DESCRIPTION: Outputs an ascii double value.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  Status  output_double( file, d )
    FILE            *file;
    double          d;
{
    Status   status;

    if( fprintf( file, " %g", d ) > 0 )
        status = OK;
    else
    {
        PRINT_ERROR( "Error outputting double.\n" );
        status = ERROR;
    }

    return( status );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : io_binary_data
@INPUT      : file
            : io_flag
            : data
            : element_size
            : n
@OUTPUT     :
@RETURNS    : Status
@DESCRIPTION: Inputs or outputs binary data, depending on io_flag.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  Status  io_binary_data( file, io_flag, data, element_size, n )
    FILE            *file;
    IO_types        io_flag;
    VOID            *data;
    size_t          element_size;
    int             n;
{
    Status   status;

    if( io_flag == READ_FILE )
        status = input_binary_data( file, data, element_size, n );
    else
        status = output_binary_data( file, data, element_size, n );

    return( status );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : io_newline
@INPUT      : file
            : io_flag
            : data
@OUTPUT     :
@RETURNS    : Status
@DESCRIPTION: Inputs or outputs an ascii or binary newline char, as appropriate.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  Status  io_newline( file, io_flag, format )
    FILE            *file;
    IO_types        io_flag;
    File_formats    format;
{
    Status   status;

    status = OK;

    if( format == ASCII_FORMAT )
    {
        if( io_flag == READ_FILE )
            status = OK;
        else
            status = output_newline( file );
    }

    return( status );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : io_quoted_string
@INPUT      : file
            : io_flag
            : format
            : str
            : str_length
@OUTPUT     :
@RETURNS    : Status
@DESCRIPTION: Inputs or outputs an ascii or binary quoted string.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  Status  io_quoted_string( file, io_flag, format, str, str_length )
    FILE            *file;
    IO_types        io_flag;
    File_formats    format;
    char            str[];
    int             str_length;
{
    int      length;
    Status   status;
    Status   io_int();
    Status   input_character();

    status = OK;

    if( format == ASCII_FORMAT )
    {
        if( io_flag == READ_FILE )
            status = input_quoted_string( file, str, str_length );
        else
            status = output_quoted_string( file, str );
    }
    else
    {
        if( io_flag == WRITE_FILE )
            length = strlen( str );

        status = io_int( file, io_flag, format, &length );

        if( io_flag == READ_FILE && length >= str_length )
        {
            PRINT( "String too large: " );
            status = ERROR;
        }

        if( status == OK )
        {
            status = io_binary_data( file, io_flag, (VOID *) str,
                                     sizeof(str[0]), length );
        }

        str[length] = (char) 0;
    }

    if( status != OK )
    {
        PRINT_ERROR( "Error in quoted string in file.\n" );
    }

    return( status );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : io_boolean
@INPUT      : file
            : io_flag
            : format
            : b              boolean value
@OUTPUT     :
@RETURNS    : Status
@DESCRIPTION: Inputs or outputs an ascii or binary boolean value.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  Status  io_boolean( file, io_flag, format, b )
    FILE            *file;
    IO_types        io_flag;
    File_formats    format;
    Boolean         *b;
{
    Status   status;

    status = OK;

    if( format == ASCII_FORMAT )
    {
        if( io_flag == READ_FILE )
            status = input_boolean( file, b );
        else
            status = output_boolean( file, *b );
    }
    else
        status = io_binary_data( file, io_flag, (VOID *) b, sizeof(*b), 1 );

    return( status );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : io_short
@INPUT      : file
            : io_flag
            : format
            : short_int              short value
@OUTPUT     :
@RETURNS    : Status
@DESCRIPTION: Inputs or outputs an ascii or binary short value.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  Status  io_short( file, io_flag, format, short_int )
    FILE            *file;
    IO_types        io_flag;
    File_formats    format;
    short           *short_int;
{
    Status   status;

    status = OK;

    if( format == ASCII_FORMAT )
    {
        if( io_flag == READ_FILE )
            status = input_short( file, short_int );
        else
            status = output_short( file, *short_int );
    }
    else
        status = io_binary_data( file, io_flag, (VOID *) short_int,
                                 sizeof(*short_int), 1 );

    return( status );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : io_unsigned_char
@INPUT      : file
            : io_flag
            : format
            : c              unsigned char value
@OUTPUT     :
@RETURNS    : Status
@DESCRIPTION: Inputs or outputs an ascii or binary unsigned char.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  Status  io_unsigned_char( file, io_flag, format, c )
    FILE            *file;
    IO_types        io_flag;
    File_formats    format;
    unsigned  char  *c;
{
    int      i;
    Status   status;

    status = OK;

    if( format == ASCII_FORMAT )
    {
        if( io_flag == READ_FILE )
        {
            if( fscanf( file, "%d", &i ) == 1 )
                *c = i;
            else
            {
                PRINT_ERROR( "Error inputting unsigned char.\n" );
                status = ERROR;
            }
        }
        else
        {
            if( fprintf( file, "%d", (int) *c ) != 1 )
            {
                PRINT_ERROR( "Error outputting unsigned char.\n" );
                status = ERROR;
            }
        }
    }
    else
        status = io_binary_data( file, io_flag, (VOID *) c, sizeof(*c), 1 );

    return( status );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : io_int
@INPUT      : file
            : io_flag
            : format
            : i              integer value
@OUTPUT     :
@RETURNS    : Status
@DESCRIPTION: Inputs or outputs an ascii or binary integer value.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  Status  io_int( file, io_flag, format, i )
    FILE            *file;
    IO_types        io_flag;
    File_formats    format;
    int             *i;
{
    Status   status;

    status = OK;

    if( format == ASCII_FORMAT )
    {
        if( io_flag == READ_FILE )
            status = input_int( file, i );
        else
            status = output_int( file, *i );
    }
    else
        status = io_binary_data( file, io_flag, (VOID *) i, sizeof(*i), 1 );

    return( status );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : io_real
@INPUT      : file
            : io_flag
            : format
            : r              real value
@OUTPUT     :
@RETURNS    : Status
@DESCRIPTION: Inputs or outputs an ascii or binary real value.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  Status  io_real( file, io_flag, format, r )
    FILE            *file;
    IO_types        io_flag;
    File_formats    format;
    Real            *r;
{
    Status   status;

    status = OK;

    if( format == ASCII_FORMAT )
    {
        if( io_flag == READ_FILE )
            status = input_real( file, r );
        else
            status = output_real( file, *r );
    }
    else
        status = io_binary_data( file, io_flag, (VOID *) r, sizeof(*r), 1 );

    return( status );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : io_double
@INPUT      : file
            : io_flag
            : format
            : d              double value
@OUTPUT     :
@RETURNS    : Status
@DESCRIPTION: Inputs or outputs an ascii or binary double value.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  Status  io_double( file, io_flag, format, d )
    FILE            *file;
    IO_types        io_flag;
    File_formats    format;
    double          *d;
{
    Status   status;

    status = OK;

    if( format == ASCII_FORMAT )
    {
        if( io_flag == READ_FILE )
            status = input_double( file, d );
        else
            status = output_double( file, *d );
    }
    else
        status = io_binary_data( file, io_flag, (VOID *) d, sizeof(*d), 1 );

    return( status );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : io_ints
@INPUT      : file
            : io_flag
            : format
            : n               number of ints
            : ints            array of ints
@OUTPUT     : 
@RETURNS    : Status
@DESCRIPTION: Inputs or outputs a list of ascii or binary integers.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  Status  io_ints( file, io_flag, format, n, ints )
    FILE            *file;
    IO_types        io_flag;
    File_formats    format;
    int             n;
    int             *ints[];
{
    Status   status;
    int      i;
#define      INTS_PER_LINE   8

    status = OK;

    if( io_flag == READ_FILE )
    {
        ALLOC( status, *ints, n );
    }

    if( status == OK )
    {
        if( format == ASCII_FORMAT )
        {
            for_less( i, 0, n )
            {
                status = io_int( file, io_flag, format, &(*ints)[i] );

                if( status == OK )
                {
                    if( i == n - 1 || (i+1) % INTS_PER_LINE == 0 )
                    {
                        status = io_newline( file, io_flag, format );
                    }
                }

                if( status == ERROR )
                {
                    break;
                }
            }
        }
        else
        {
            status = io_binary_data( file, io_flag, (VOID *) *ints,
                                     sizeof((*ints)[0]), n );
        }
    }

    return( status );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : io_unsigned_chars
@INPUT      : file
            : io_flag
            : format
            : n               number of unsigned chars
            : unsigned_chars  array of unsigned chars
@OUTPUT     : 
@RETURNS    : Status
@DESCRIPTION: Inputs or outputs a list of ascii or binary unsigned chars.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  Status  io_unsigned_chars( file, io_flag, format, n, unsigned_chars )
    FILE            *file;
    IO_types        io_flag;
    File_formats    format;
    int             n;
    unsigned char   *unsigned_chars[];
{
    Status   status;
    int      i;

    status = OK;

    if( io_flag == READ_FILE )
        ALLOC( status, *unsigned_chars, n );

    if( status == OK )
    {
        if( format == ASCII_FORMAT )
        {
            for_less( i, 0, n )
            {
                status = io_unsigned_char( file, io_flag, format,
                                           &(*unsigned_chars)[i] );

                if( status == OK )
                {
                    if( i == n - 1 || (i+1) % INTS_PER_LINE == 0 )
                        status = io_newline( file, io_flag, format );
                }

                if( status == ERROR )
                    break;
            }
        }
        else
        {
            status = io_binary_data( file, io_flag, (VOID *) (*unsigned_chars),
                                     sizeof((*unsigned_chars)[0]), n );
        }
    }

    return( status );
}
