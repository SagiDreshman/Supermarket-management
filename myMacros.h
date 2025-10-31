#ifndef MYMACROS_H
#define MYMACROS_H


#define CHECK_RETURN_0(ptr) if(!ptr) return 0
#define CHECK_MSG_RETURN_0(ptr) if(!ptr) { printf("Error in open file\n"); return 0; }
#define FREE_CLOSE_FILE_RETURN_0(ptr, fp) {free(ptr); fclose(fp); return 0;}
#define CLOSE_RETURN_0(fp) if(fp) {fclose(fp); return 0;}

#endif