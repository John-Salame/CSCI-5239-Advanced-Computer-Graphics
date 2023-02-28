#include "CSCIx239.h"
int getSizeOfGrassVBO(int numBlades);
int getNumVerticesPerGrass();
void copyGrassData(float* dest, int numBlades);
void InitGrassVBO(float data[], int dataSize, unsigned int* grassVbo);
void InitGrassVAO(int shader, unsigned int* grassVbo, unsigned int* grassVao);
void bladeOfGrass(double height);
