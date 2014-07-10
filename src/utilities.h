#ifndef ___UTILITIES_H___
#define ___UTILITIES_H___

#define PRINT_ERROR(str) fprintf(stderr,"%s",str)

#define PRINT_ERROR_AND_RETURN_NULL_IF_NULL(ptr,str)			\
	if (NULL == ptr){																				\
	fprintf(stderr,"%s",str);																\
	return NULL;																						\
	}

#define PRINT_ERROR_AND_RETURN_NEG_ONE_IF_NULL(ptr,str)	\
	if (NULL == ptr){																			\
	fprintf(stderr,"%s",str);															\
	return -1;																						\
	}

#endif /*___UTILITIES_H___*/
