
#include  <def_string.h>
#include  <def_mni.h>
#include  <def_files.h>
#include  <stdlib.h>

/* ----------------------------- MNI Header -----------------------------------
@NAME       : alloc_check.c
@INPUT      : 
@OUTPUT     : 
@RETURNS    : 
@DESCRIPTION: Maintains a skiplist structure to list all memory allocated,
            : and check for errors such as freeing a pointer twice or
            : overlapping allocations.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

#define  MAX_SKIP_LEVELS   50
#define  SKIP_P            0.5

#define  MEMORY_DIFFERENCE  1000000

static    void     update_total_memory();
static    int      get_random_level();
static    Real     get_random_0_to_1();
static    void     output_entry();
static    Real     current_seconds();
static    void     get_date();
void               abort_if_allowed();

typedef  struct skip_struct
{
    void                    *ptr;
    int                     n_bytes;
    char                    *source_file;
    int                     line_number;
    Real                    time_of_alloc;
    struct  skip_struct     *forward[1];
} skip_struct;

typedef  struct
{
    int            next_memory_threshold;
    int            total_memory_allocated;
    skip_struct    *header;
    int            level;
} alloc_struct;

typedef  struct
{
    skip_struct   *update[MAX_SKIP_LEVELS];
} update_struct;

#ifdef sgi
typedef  size_t    alloc_int;
typedef  void      *alloc_ptr;
#else
typedef  unsigned  alloc_int;
typedef  char      *alloc_ptr;
#endif

#define  ALLOC_SKIP_STRUCT( ptr, n_level )                                    \
     (ptr) = (skip_struct *) malloc( (alloc_int)                              \
                 (sizeof(skip_struct)+((n_level)-1) * sizeof(skip_struct *)) );

/* ----------------------------- MNI Header -----------------------------------
@NAME       : initialize_alloc_list
@INPUT      : alloc_list
@OUTPUT     : 
@RETURNS    : 
@DESCRIPTION: Initializes the allocation list to empty.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

private   void  initialize_alloc_list( alloc_list )
    alloc_struct  *alloc_list;
{
    int   i;

    alloc_list->next_memory_threshold = MEMORY_DIFFERENCE;
    alloc_list->total_memory_allocated = 0;

    ALLOC_SKIP_STRUCT( alloc_list->header, MAX_SKIP_LEVELS );
    alloc_list->level = 1;

    for_less( i, 0, MAX_SKIP_LEVELS )
        alloc_list->header->forward[i] = (skip_struct *) 0;
}

private  void  check_initialized_alloc_list( alloc_list )
    alloc_struct  *alloc_list;
{
    static   Boolean  first = TRUE;

    if( first )
    {
        first = FALSE;
        initialize_alloc_list( alloc_list );
    }
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : find_pointer_position
@INPUT      : alloc_list
            : ptr
@OUTPUT     : update
@RETURNS    : TRUE if found
@DESCRIPTION: Searches the alloc_list for the given ptr, and sets the update
            : struct so that it can provide an insert.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

private  Boolean  find_pointer_position( alloc_list, ptr, update )
    alloc_struct    *alloc_list;
    void            *ptr;
    update_struct   *update;
{
    int           i;
    skip_struct   *x;
    Boolean       found;

    x = alloc_list->header;

    for( i = alloc_list->level-1;  i >= 0;  --i )
    {
        while( x->forward[i] != (skip_struct *) 0 && x->forward[i]->ptr < ptr )
        {
            x = x->forward[i];
        }
        update->update[i] = x;
    }

    x = update->update[0]->forward[0];

    found = (x != (skip_struct *) 0) && (x->ptr == ptr);

    return( found );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : insert_ptr_in_alloc_list
@INPUT      : alloc_list
            : update           - the set of pointers indicating where to insert
            : ptr              }
            : n_bytes          }}
            : source_file      }}} these are recorded in the list
            : line_number      }}
            : time_of_alloc    }
@OUTPUT     : 
@RETURNS    : 
@DESCRIPTION: Records the allocated pointer in the allocation list.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

private   void  insert_ptr_in_alloc_list( alloc_list, update, ptr, n_bytes,
                                          source_file, line_number,
                                          time_of_alloc )
    alloc_struct   *alloc_list;
    update_struct  *update;
    void           *ptr;
    int            n_bytes;
    char           source_file[];
    int            line_number;
    Real           time_of_alloc;
{
    int           i, new_level;
    skip_struct   *x;

    new_level = get_random_level();

    if( new_level > alloc_list->level )
    {
        for( i = alloc_list->level;  i < new_level;  ++i )
            update->update[i] = alloc_list->header;

        alloc_list->level = new_level;
    }

    ALLOC_SKIP_STRUCT( x, new_level );

    x->ptr = ptr;
    x->n_bytes = n_bytes;
    x->source_file = source_file;
    x->line_number = line_number;
    x->time_of_alloc = time_of_alloc;
    update_total_memory( alloc_list, n_bytes );

    for( i = 0;  i < new_level;  ++i )
    {
        x->forward[i] = update->update[i]->forward[i];
        update->update[i]->forward[i] = x;
    }
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : check_overlap
@INPUT      : update
            : ptr
            : n_bytes
@OUTPUT     : entry
@RETURNS    : TRUE if an overlap
@DESCRIPTION: Checks the new ptr to see if it overlaps with the previous and
            : following memory allocations in the list, and returns the result.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

private  Boolean  check_overlap( update, ptr, n_bytes, entry )
    update_struct   *update;
    void            *ptr;
    int             n_bytes;
    skip_struct     **entry;
{
    Boolean      overlap;

    overlap = FALSE;

    *entry = update->update[0];

    if( *entry != (skip_struct *) 0 )
    {
        if( (void *) ((char *) (*entry)->ptr + (*entry)->n_bytes) > ptr )
             overlap = TRUE;
        else
        {
            (*entry) = (*entry)->forward[0];
            if( *entry != (skip_struct *) 0 &&
                (void *) ((char*)ptr + n_bytes) > (*entry)->ptr )
                overlap = TRUE;
        }
    }

    return( overlap );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : remove_ptr_from_alloc_list
@INPUT      : alloc_list
            : ptr
@OUTPUT     : source_file
            : line_number
            : time_of_alloc
@RETURNS    : TRUE if it existed
@DESCRIPTION: Finds and deletes the entry in the skip list associated with
            : ptr, and returns the information associated with the entry
            : (source_file, line_number, time_of_alloc).
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

private   Boolean  remove_ptr_from_alloc_list( alloc_list, ptr, source_file,
                                           line_number, time_of_alloc )
    alloc_struct   *alloc_list;
    void           *ptr;
    char           *source_file[];
    int            *line_number;
    Real           *time_of_alloc;
{
    int           i;
    Boolean       found;
    skip_struct   *x;
    update_struct update;

    found = find_pointer_position( alloc_list, ptr, &update );

    if( found )
    {
        x = update.update[0]->forward[0];

        *source_file = x->source_file;
        *line_number = x->line_number;
        *time_of_alloc = x->time_of_alloc;

        update_total_memory( alloc_list, -x->n_bytes );

        for( i = 0;  i < alloc_list->level;  ++i )
        {
            if( update.update[i]->forward[i] != x )
                break;
            update.update[i]->forward[i] = x->forward[i];
        }

        free( (alloc_ptr) x );

        while( alloc_list->level > 1 &&
               alloc_list->header->forward[alloc_list->level-1] ==
                    (skip_struct *) 0 )
        {
            --alloc_list->level;
        }
    }

    return( found );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : get_random_level
@INPUT      : 
@OUTPUT     : 
@RETURNS    : a random level between 1 and MAX_LEVELS
@DESCRIPTION: Determines a random level with exponential probability of higher
            : levels.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

private  int  get_random_level()
{
    int    level;
    Real   get_random_0_to_1();

    level = 1;

    while( get_random_0_to_1() < SKIP_P && level < MAX_SKIP_LEVELS )
        ++level;

    return( level );
}

#ifdef  NOT_NEEDED
private   void  delete_alloc_list( alloc_list )
    alloc_struct  *alloc_list;

{
    skip_struct   *ptr, *deleting;

    ptr = alloc_list->header;

    while( ptr != (skip_struct *) 0 )
    {
        deleting = ptr;
        ptr = ptr->forward[0];
        free( (void *) deleting );
    }
}
#endif

/* ----------------------------- MNI Header -----------------------------------
@NAME       : output_alloc_list
@INPUT      : file
            : alloc_list
@OUTPUT     : 
@RETURNS    : 
@DESCRIPTION: Outputs the list of allocated memory to the file.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

private  void  output_alloc_list( file, alloc_list )
    FILE          *file;
    alloc_struct  *alloc_list;
{
    skip_struct  *ptr;

    ptr = alloc_list->header->forward[0];

    while( ptr != (skip_struct *) 0 )
    {
        output_entry( file, ptr );
        ptr = ptr->forward[0];
    }
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : update_total_memory
@INPUT      : alloc_list
            : n_bytes
@OUTPUT     : 
@RETURNS    : 
@DESCRIPTION: Adds n_bytes to the size of memory recorded.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

private  void  update_total_memory( alloc_list, n_bytes )
    alloc_struct  *alloc_list;
    int           n_bytes;
{
    alloc_list->total_memory_allocated += n_bytes;

    if( size_display_enabled() &&
        alloc_list->total_memory_allocated >
        alloc_list->next_memory_threshold )
    {
        alloc_list->next_memory_threshold = MEMORY_DIFFERENCE *
                (alloc_list->total_memory_allocated / MEMORY_DIFFERENCE + 1);
        (void) printf( "Memory allocated =%5.1f Megabytes\n",
                       (Real) alloc_list->total_memory_allocated / 1000000.0 );
    }
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : print_source_location
@INPUT      : source_file
            : line_number
            : time_of_alloc
@OUTPUT     : 
@RETURNS    : 
@DESCRIPTION: Prints the information about a particular allocation.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

private  void  print_source_location( source_file, line_number,
                                      time_of_alloc )
    char   source_file[];
    int    line_number;
    Real   time_of_alloc;
{
    (void) printf( "%s:%d\t%g seconds", source_file, line_number,
                   time_of_alloc );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : output_entry
@INPUT      : file
            : entry
@OUTPUT     : 
@RETURNS    : 
@DESCRIPTION: Outputs the information about an allocation entry to the file.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

private  void  output_entry( file, entry )
    FILE          *file;
    skip_struct   *entry;
{
    (void) fprintf( file, "%s:%d\t%g seconds\n",
                    entry->source_file,
                    entry->line_number,
                    entry->time_of_alloc );
}

/*  
--------------------------------------------------------------------------
    Routines that are to be called from outside this file
--------------------------------------------------------------------------
*/

private   alloc_struct   alloc_list;

/* ----------------------------- MNI Header -----------------------------------
@NAME       : get_total_memory_alloced
@INPUT      : 
@OUTPUT     : 
@RETURNS    : int  - the number of bytes allocated
@DESCRIPTION: Returns the total amount of memory allocated by the program,
            : not counting that used by the skip list.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  int  get_total_memory_alloced()
{
    return( alloc_list.total_memory_allocated );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : alloc_checking_enabled
@INPUT      : 
@OUTPUT     : 
@RETURNS    : TRUE if alloc checking is turned on
@DESCRIPTION: Checks an environment variable to see if alloc checking is
            : not disabled.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

private  Boolean  alloc_checking_enabled()
{
#ifdef NO_DEBUG_ALLOC
    return( FALSE );
#else
    static  Boolean  first = TRUE;
    static  Boolean  enabled;

    if( first )
    {
        enabled = !ENV_EXISTS( "NO_DEBUG_ALLOC" );
        first = FALSE;
    }

    return( enabled );
#endif
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : size_display_enabled
@INPUT      : 
@OUTPUT     : 
@RETURNS    : TRUE if size displaying is turned on
@DESCRIPTION: Checks an environment variable to see if memory size display
            : is disabled.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

private  Boolean  size_display_enabled()
{
#ifdef NO_DEBUG_ALLOC
    return( FALSE );
#else
    static  Boolean  first = TRUE;
    static  Boolean  enabled;

    if( first )
    {
        enabled = ENV_EXISTS( "ALLOC_SIZE" );
        first = FALSE;
    }

    return( enabled );
#endif
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : record_ptr
@INPUT      : ptr
            : n_bytes
            : source_file
            : line_number
@OUTPUT     : 
@RETURNS    : 
@DESCRIPTION: Records the information about a single allocation in the list.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  void  record_ptr( ptr, n_bytes, source_file, line_number )
    void   *ptr;
    int    n_bytes;
    char   source_file[];
    int    line_number;
{
    Real           current_time;
    update_struct  update_ptrs;
    void           check_initialized_alloc_list();
    skip_struct    *entry;

    if( alloc_checking_enabled() )
    {
        current_time = current_seconds();

        check_initialized_alloc_list( &alloc_list );

        if( n_bytes <= 0 )
        {
            print_source_location( source_file, line_number, current_time );
            (void) printf( ": Alloc called with zero size.\n" );
            abort_if_allowed();
        }
        else if( ptr == (void *) 0 )
        {
            print_source_location( source_file, line_number, current_time );
            (void) printf( ": Alloc returned a NIL pointer.\n" );
            abort_if_allowed();
        }
        else
        {
            (void) find_pointer_position( &alloc_list, ptr, &update_ptrs );

            if( check_overlap( &update_ptrs, ptr, n_bytes, &entry ) )
            {
                print_source_location( source_file, line_number, current_time );
                (void) printf(
                 ": Alloc returned a pointer overlapping an existing block:\n"
                 );
                print_source_location( entry->source_file, entry->line_number,
                                       entry->time_of_alloc );
                (void) printf( "\n" );
                abort_if_allowed();
            }
            else
                insert_ptr_in_alloc_list( &alloc_list,
                           &update_ptrs, ptr, n_bytes,
                           source_file, line_number, current_time );
        }
    }
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : change_ptr
@INPUT      : old_ptr
            : new_ptr
            : n_bytes
            : source_file
            : line_number
@OUTPUT     : 
@RETURNS    : 
@DESCRIPTION: Changes the information (mainly the n_bytes) associated with a
            : given pointer.  This function is called from the def_alloc
            : macros after a realloc().
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  void  change_ptr( old_ptr, new_ptr, n_bytes, source_file, line_number )
    void   *old_ptr;
    void   *new_ptr;
    int    n_bytes;
    char   source_file[];
    int    line_number;
{
    char           *orig_source;
    int            orig_line;
    Real           time_of_alloc;
    skip_struct    *entry;
    update_struct  update_ptrs;

    if( alloc_checking_enabled() )
    {
        check_initialized_alloc_list( &alloc_list );

        if( n_bytes <= 0 )
        {
            print_source_location( source_file, line_number,
                                   current_seconds() );
            (void) printf( ": Realloc called with zero size.\n" );
            abort_if_allowed();
        }
        else if( !remove_ptr_from_alloc_list( &alloc_list, old_ptr,
                      &orig_source, &orig_line, &time_of_alloc ) )
        {
            print_source_location( source_file, line_number,
                                   current_seconds() );
            (void) printf(
                     ": Tried to realloc a pointer not already alloced.\n");
            abort_if_allowed();
        }
        else
        {
            (void) find_pointer_position( &alloc_list, new_ptr, &update_ptrs );

            if( check_overlap( &update_ptrs, new_ptr, n_bytes, &entry ) )
            {
                print_source_location( source_file, line_number,
                                       current_seconds());
                (void) printf(
               ": Realloc returned a pointer overlapping an existing block:\n");
                print_source_location( entry->source_file, entry->line_number,
                                       entry->time_of_alloc );
                (void) printf( "\n" );
                abort_if_allowed();
            }
            else
                insert_ptr_in_alloc_list( &alloc_list,
                       &update_ptrs, new_ptr, n_bytes,
                       orig_source, orig_line, time_of_alloc );
        }
    }
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : unrecord_ptr
@INPUT      : ptr
            : source_file
            : line_number
@OUTPUT     : 
@RETURNS    : TRUE if ptr was in list
@DESCRIPTION: Removes the entry for the given ptr from the list.  Called by
            : the macros during a FREE.  Returns TRUE if the pointer was
            : in the list.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  Boolean  unrecord_ptr( ptr, source_file, line_number )
    void   *ptr;
    char   source_file[];
    int    line_number;
{
    Boolean  was_previously_alloced;
    char     *orig_source;
    int      orig_line;
    Real     time_of_alloc;

    was_previously_alloced = TRUE;

    if( alloc_checking_enabled() )
    {
        check_initialized_alloc_list( &alloc_list );

        if( ptr == (void *) 0 )
        {
            print_source_location( source_file, line_number,
                                   current_seconds() );
            (void) printf( ": Tried to free a NIL pointer.\n" );
            abort_if_allowed();
            was_previously_alloced = FALSE;
        }
        else if( !remove_ptr_from_alloc_list( &alloc_list, ptr, &orig_source,
                                              &orig_line, &time_of_alloc ) )
        {
            print_source_location( source_file, line_number,
                                   current_seconds() );
            (void) printf( ": Tried to free a pointer not alloced.\n" );
            abort_if_allowed();
            was_previously_alloced = FALSE;
        }
    }

    return( was_previously_alloced );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : output_alloc_to_file
@INPUT      : filename
@OUTPUT     : 
@RETURNS    : 
@DESCRIPTION: Outputs a list of all memory allocated to the given file.  Usually
            : done at the end of the program to see if there is any memory that
            : was orphaned.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  void  output_alloc_to_file( filename )
    char   filename[];
{
    FILE     *file;
    void     output_alloc_list();
    String   date_str;

    if( alloc_checking_enabled() )
    {
        check_initialized_alloc_list( &alloc_list );

        if( filename != (char *) 0 && filename[0] != (char) 0 )
            file = fopen( filename, "w" );
        else
            file = stdout;

        if( file != (FILE *) 0 )
        {
            get_date( date_str );

            (void) fprintf( file, "Alloc table at %s\n", date_str );

            output_alloc_list( file, &alloc_list );

            if( file != stdout )
                (void) fclose( file );
        }
    }
}

#include  <sys/time.h>
#include  <def_string.h>

/* ----------------------------- MNI Header -----------------------------------
@NAME       : get_date
@INPUT      : 
@OUTPUT     : date_str
@RETURNS    : 
@DESCRIPTION: Fills in the date into the string.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

private  void  get_date( date_str )
    char  date_str[];
{
    time_t           clock_time;
    struct  tm       *time_tm;
    char             *str;
#ifndef sgi
    time_t time();
#endif

    (void) time( &clock_time );

    time_tm = localtime( &clock_time );

    str = asctime( time_tm );

    (void) strcpy( date_str, str );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : current_seconds
@INPUT      : 
@OUTPUT     : 
@RETURNS    : Real
@DESCRIPTION: Returns the number of seconds since the first call to this.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

private  Real  current_seconds()
{
    static  Boolean          first_call = TRUE;
    static  struct  timeval  first;
    struct  timeval          current;
    Real                     secs;

    if( first_call )
    {
        first_call = FALSE;
        (void) gettimeofday( &first, (struct timezone *) 0 );
        secs = 0.0;
    }
    else
    {
        (void) gettimeofday( &current, (struct timezone *) 0 );
        secs = (double) current.tv_sec - (double) first.tv_sec +
               1.0e-6 * (double) (current.tv_usec - first.tv_usec);
    }

    return( secs );
}

#include  <def_math.h>

#define  MAX_RAND  2147483648.0

private  Boolean  initialized = FALSE;

/* ----------------------------- MNI Header -----------------------------------
@NAME       : set_random_seed
@INPUT      : seed
@OUTPUT     : 
@RETURNS    : 
@DESCRIPTION: Initializes the random number generation.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

private  void  set_random_seed( seed )
    int   seed;
{
#ifdef sgi
    (void) srandom( (unsigned int) seed );
#else
    int  srandom();
    (void) srandom( seed );
#endif

    initialized = TRUE;
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : check_initialized
@INPUT      : 
@OUTPUT     : 
@RETURNS    : 
@DESCRIPTION: Checks if the random number generator initialized.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

private  void  check_initialized()
{
    struct   timeval   t;
    int                seed;

    if( !initialized )
    {
        (void) gettimeofday( &t, (struct timezone *) 0 );

        seed = (int) t.tv_usec;

        set_random_seed( seed );
    }
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : get_random
@INPUT      : 
@OUTPUT     : 
@RETURNS    : int
@DESCRIPTION: Gets a random integer.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

private  int  get_random()
{
    check_initialized();

#ifdef sgi
    return( random() );
#else
    {
        long   random();

        return( (int) random() );
    }
#endif
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : get_random_0_to_1
@INPUT      : 
@OUTPUT     : 
@RETURNS    : Real
@DESCRIPTION: Returns a random number between 0 and 1.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    :                      David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

private  Real  get_random_0_to_1()
{
    return( (Real) get_random() / MAX_RAND );
}
