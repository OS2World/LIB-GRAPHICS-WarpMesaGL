/* Debug_mem.h */

#define CHECK_MEM  {int  rc = _heapchk();\
    if (_HEAPOK != rc)               \
     { switch(rc) {                  \
          case _HEAPEMPTY:  printf("The heap has not been initialized in %i of %s",__LINE__,__FUNCTION__);\
             break;                                                    \
          case _HEAPBADNODE: printf("A memory node is corrupted or the heap is damaged in %i of %s",__LINE__, __FUNCTION__);\
             break;                                                                     \
          case _HEAPBADBEGIN: printf("The heap specified is not valid in %i of %s",__LINE__,__FUNCTION__);                 \
             break;                                                                     \
       }                                                                                \
       exit(rc);                                                                        \  
    }                                                                                   \
                    }
