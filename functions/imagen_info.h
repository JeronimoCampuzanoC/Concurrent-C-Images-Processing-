#ifndef IMAGEN_INFO_H
#define IMAGEN_INFO_H


typedef struct
{
    int ancho;                
    int alto;                
    int canales;              
    unsigned char ***pixeles;
} ImagenInfo;

#endif // IMAGEN_INFO_H