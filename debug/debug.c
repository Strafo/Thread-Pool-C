#include"debug.h"

#ifdef _DEBUG
inline void debug(FILE* fp,char* message){
	fprintf(fp, "%s",message );
}
inline void debugInt(FILE* fp,int number){
	fprintf(fp, "%d",number );
}
#else
inline void debug(FILE* fp,char* message){};
inline void debugInt(FILE* fp,int number){};
#endif//_DEBUG