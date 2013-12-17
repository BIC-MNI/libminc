#include <stdio.h>
#include <hdf5.h>
#include "minc2.h"

int
main(int argc, char **argv)
{
    miclass_t myclass = MI_CLASS_REAL;
    mitype_t mytype = MI_TYPE_UNKNOWN;
    misize_t mysize = 0;
    char *myname = NULL;
    mihandle_t volume = NULL;

    /* Turn off automatic error reporting.
     */
    H5Eset_auto2(H5E_DEFAULT, NULL, NULL);

    /* Check each file.
     */
    while (--argc > 0) {
	
	++argv;

	if (micreate_volume(*argv, 0, NULL, 0, 0, NULL, &volume) < 0) {
	    fprintf(stderr, "Error opening %s\n", *argv);
	}
	else {
	    int i;
	    /* Repeat many times to expose resource leakage problems, etc.
	     */
	    for (i = 0; i < 25000; i++) {
		miget_data_type(volume, &mytype);
		miget_data_type_size(volume, &mysize);
		miget_data_class(volume, &myclass);
		miget_space_name(volume, &myname);
	    
		mifree_name(myname);
	    }

	    miclose_volume(volume);

	    printf("file: %s type %d size %lld class %d name %s\n", *argv,
		   mytype, mysize, myclass, myname);
	}

    }
    return (0);
}

