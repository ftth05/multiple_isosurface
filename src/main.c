#ifdef __APPLE__
#include <GLUT/glut.h>
#else

#include <GL/glut.h>

#endif

# include <stdio.h>
# include <stdlib.h>
# include <math.h>
//# include <conio.h>
# include <stdarg.h>
# include <string.h>
# include <time.h>
# include <limits.h>
# include "isosurface.h"
# include <errno.h>

int flag = 0;


#define LEFT_MOUSE_BUTTON_BIT   1
#define MIDDLE_MOUSE_BUTTON_BIT 2
#define RIGHT_MOUSE_BUTTON_BIT  4

int mbs;
int mouse_prev_x, mouse_prev_y;
int moving = 0;

static float angle = 0.0;
GLfloat material[] = {0.0, 0, 0.5};

char key;
//* Transformation states
float rot[3], trans[3];
float rot_inc[3], trans_inc[3];
float rot_matrix[3];
double rot_matrix_int[3];
double rot_matrix_inc[3];

void processSpecialKeys(int key, int x, int y);

void SetMaterial(GLfloat mat[4]) {
    int i;
    for (i = 0; i < 4; i++)
        curMat[i] = mat[i];
}

NODE *MarchingCube(float ***dataset, float isoValue, int maxX, int maxY, int maxZ, NODE *list) {
    int i, j, k, m, num, inc = 1;
    int numOfTriangles;
    float d, norm[3], dx1, dx2, dy1, dy2, dz1, dz2;
    XYZ n[3];
    GRIDCELL v;
    TRIANGLE t[5];
    NODE *tmpNode;
    //printf("isovalue = %f,  maxX = %d, maxY = %d, maxZ = %d\n",isoValue, maxX, maxY, maxZ);
    //x = clock();
    for (i = 0; i < maxX - inc; i += inc) {
        for (j = 0; j < maxY - inc; j += inc) {
            for (k = 0; k < maxZ - inc; k += inc) {
                v.val[0] = dataset[i][j][k];
                v.p[0].x = i * 1.0;
                v.p[0].y = j * 1.0;
                v.p[0].z = k * 1.0;

                v.val[1] = dataset[i + inc][j][k];
                v.p[1].x = i + inc * 1.0;
                v.p[1].y = j * 1.0;
                v.p[1].z = k * 1.0;

                v.val[2] = dataset[i + inc][j + inc][k];
                v.p[2].x = i + inc * 1.0;
                v.p[2].y = j + inc * 1.0;
                v.p[2].z = k * 1.0;

                v.val[3] = dataset[i][j + inc][k];
                v.p[3].x = i * 1.0;
                v.p[3].y = j + inc * 1.0;
                v.p[3].z = k * 1.0;

                v.val[4] = dataset[i][j][k + inc];
                v.p[4].x = i * 1.0;
                v.p[4].y = j * 1.0;
                v.p[4].z = k + inc * 1.0;

                v.val[5] = dataset[i + inc][j][k + inc];
                v.p[5].x = i + inc * 1.0;
                v.p[5].y = j * 1.0;
                v.p[5].z = k + inc * 1.0;

                v.val[6] = dataset[i + inc][j + inc][k + inc];
                v.p[6].x = i + inc * 1.0;
                v.p[6].y = j + inc * 1.0;
                v.p[6].z = k + inc * 1.0;

                v.val[7] = dataset[i][j + inc][k + inc];
                v.p[7].x = i * 1.0;
                v.p[7].y = j + inc * 1.0;
                v.p[7].z = k + inc * 1.0;

                numOfTriangles = Polygonise(v, isoValue, t);
                //printf(" Number of Triangles = %d \n", numOfTriangles);
                for (m = 0; m < numOfTriangles; m++) {
                    dx1 = t[m].p[1].x - t[m].p[0].x;
                    dy1 = t[m].p[1].y - t[m].p[0].y;
                    dz1 = t[m].p[1].z - t[m].p[0].z;

                    dx2 = t[m].p[2].x - t[m].p[0].x;
                    dy2 = t[m].p[2].y - t[m].p[0].y;
                    dz2 = t[m].p[2].z - t[m].p[0].z;

                    norm[0] = dy1 * dz2 - dz1 * dy2;
                    norm[1] = dz1 * dx2 - dx1 * dz2;
                    norm[2] = dx1 * dy2 - dy1 * dx2;
                    d = sqrt(norm[0] * norm[0] + norm[1] * norm[1] + norm[2] * norm[2]);
                    norm[0] /= d;
                    norm[1] /= d;
                    norm[2] /= d;
                    for (num = 0; num < 3; num++) {
                        n[num].x = norm[0];
                        n[num].y = norm[1];
                        n[num].z = norm[2];
                    }
                    tmpNode = (NODE *) malloc(sizeof(NODE));
                    for (num = 0; num < 3; num++) {
                        tmpNode->t.p[num].x = t[m].p[num].x / maxX;
                        tmpNode->t.p[num].y = t[m].p[num].y / maxY;
                        tmpNode->t.p[num].z = t[m].p[num].z / maxZ;
                        tmpNode->n[num] = n[num];
                    }
                    tmpNode->depth = GetDepth(t[m]);
                    for (num = 0; num < 4; num++)
                        tmpNode->mat[num] = curMat[num];
                    tmpNode->next = NULL;
                    //printf("Calling insert\n");
                    list = Insert(list, tmpNode);
                }
            }
        }
    }
    //time_in_seconds = (double) (clock() - x)/CLOCKS_PER_SEC;
    //printf("%f", time_in_seconds);
    return list;
}

NODE *Insert(NODE *listset, NODE *tmpNode) {
    NODE *temp;
    if (listset == NULL) return tmpNode;
    else {
        tmpNode->next = listset->next;
        listset->next = tmpNode;
        return listset;
    }
    temp = listset;
    if (temp->next == NULL) {
        if (temp->depth > tmpNode->depth) temp->next = tmpNode;
        else {
            tmpNode->next = temp;
            listset = tmpNode;
        }
        return listset;
    }

    while (temp->next != NULL) {
        if (temp->next->depth > tmpNode->depth) temp = temp->next;
        else {
            tmpNode->next = temp->next;
            temp->next = tmpNode;
            return listset;
        }
    }
    temp->next = tmpNode;
    return listset;

}

float GetDepth(TRIANGLE t) {
    float z;
    z = t.p[0].z;
    z = t.p[1].z > z ? t.p[1].z : z;
    z = t.p[2].z > z ? t.p[2].z : z;
    return z;
}

NODE *DeleteList(NODE *listset) {
    NODE *temp;
    while (listset) {
        temp = listset->next;
        free(listset);
        listset = NULL;
        //delete(listset);
        listset = temp;
    }
    return listset;
}

void DrawIsoSurface(NODE *listset) {
    int i;
    XYZ n, p;
    NODE *temp;
    temp = listset;
    glPushMatrix();
    glBegin(GL_TRIANGLES);
    while (temp != NULL) {
        //glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, temp->mat);
        //glMaterialfv(GL_FRONT_AND_BACK, GL_COLOR_INDEXES, temp->mat);
        for (i = 0; i < 3; i++) {
            n = temp->n[i];
            p = temp->t.p[i];
            glNormal3f(n.x, n.y, n.z);
            glVertex3f(p.x, p.y, p.z);
        }
        temp = temp->next;
    }
    glEnd();

    glPopMatrix();
}

/*
	Source: http://astronomy.swin.edu.au/~pbourke/modelling/polygonise/
	Written by Paul Bourke
	May 1997 
   Given a grid cell and an isolevel, calculate the triangular
   facets required to represent the isosurface through the cell.
   Return the number of triangular facets, the array "triangles"
   will be loaded up with the vertices at most 5 triangular facets.
	0 will be returned if the grid cell is either totally above
   of totally below the isolevel.
*/
int Polygonise(GRIDCELL grid, float isolevel, TRIANGLE *triangles) {
    int i, ntriang;
    int cubeindex;
    XYZ vertlist[12];
    /*
       Determine the index into the edge table which
       tells us which vertices are inside of the surface
    */
    cubeindex = 0;
    if (grid.val[0] < isolevel) cubeindex |= 1;
    if (grid.val[1] < isolevel) cubeindex |= 2;
    if (grid.val[2] < isolevel) cubeindex |= 4;
    if (grid.val[3] < isolevel) cubeindex |= 8;
    if (grid.val[4] < isolevel) cubeindex |= 16;
    if (grid.val[5] < isolevel) cubeindex |= 32;
    if (grid.val[6] < isolevel) cubeindex |= 64;
    if (grid.val[7] < isolevel) cubeindex |= 128;

    /* Cube is entirely in/out of the surface */
    if (edgeTable[cubeindex] == 0)
        return (0);

    /* Find the vertices where the surface intersects the cube */
    if (edgeTable[cubeindex] & 1)
        vertlist[0] =
                VertexInterp(isolevel, grid.p[0], grid.p[1], grid.val[0], grid.val[1]);
    if (edgeTable[cubeindex] & 2)
        vertlist[1] =
                VertexInterp(isolevel, grid.p[1], grid.p[2], grid.val[1], grid.val[2]);
    if (edgeTable[cubeindex] & 4)
        vertlist[2] =
                VertexInterp(isolevel, grid.p[2], grid.p[3], grid.val[2], grid.val[3]);
    if (edgeTable[cubeindex] & 8)
        vertlist[3] =
                VertexInterp(isolevel, grid.p[3], grid.p[0], grid.val[3], grid.val[0]);
    if (edgeTable[cubeindex] & 16)
        vertlist[4] =
                VertexInterp(isolevel, grid.p[4], grid.p[5], grid.val[4], grid.val[5]);
    if (edgeTable[cubeindex] & 32)
        vertlist[5] =
                VertexInterp(isolevel, grid.p[5], grid.p[6], grid.val[5], grid.val[6]);
    if (edgeTable[cubeindex] & 64)
        vertlist[6] =
                VertexInterp(isolevel, grid.p[6], grid.p[7], grid.val[6], grid.val[7]);
    if (edgeTable[cubeindex] & 128)
        vertlist[7] =
                VertexInterp(isolevel, grid.p[7], grid.p[4], grid.val[7], grid.val[4]);
    if (edgeTable[cubeindex] & 256)
        vertlist[8] =
                VertexInterp(isolevel, grid.p[0], grid.p[4], grid.val[0], grid.val[4]);
    if (edgeTable[cubeindex] & 512)
        vertlist[9] =
                VertexInterp(isolevel, grid.p[1], grid.p[5], grid.val[1], grid.val[5]);
    if (edgeTable[cubeindex] & 1024)
        vertlist[10] =
                VertexInterp(isolevel, grid.p[2], grid.p[6], grid.val[2], grid.val[6]);
    if (edgeTable[cubeindex] & 2048)
        vertlist[11] =
                VertexInterp(isolevel, grid.p[3], grid.p[7], grid.val[3], grid.val[7]);

    /* Create the triangle */
    ntriang = 0;
    for (i = 0; triTable[cubeindex][i] != -1; i += 3) {
        triangles[ntriang].p[0] = vertlist[triTable[cubeindex][i]];
        triangles[ntriang].p[1] = vertlist[triTable[cubeindex][i + 1]];
        triangles[ntriang].p[2] = vertlist[triTable[cubeindex][i + 2]];
        ntriang++;
    }

    return (ntriang);
}

/*
   Linearly interpolate the position where an isosurface cuts
   an edge between two vertices, each with their own scalar value
*/
XYZ VertexInterp(float isolevel, XYZ p1, XYZ p2, float valp1, float valp2) {
    float mu;
    XYZ p;

    if (fabs(isolevel - valp1) < 0.00001)
        return (p1);
    if (fabs(isolevel - valp2) < 0.00001)
        return (p2);
    if (fabs(valp1 - valp2) < 0.00001)
        return (p1);
    mu = (isolevel - valp1) / (valp2 - valp1);
    p.x = p1.x + mu * (p2.x - p1.x);
    p.y = p1.y + mu * (p2.y - p1.y);
    p.z = p1.z + mu * (p2.z - p1.z);

    return (p);
}

int main(int argc, char **argv) {
    x = clock();
    memset(rot, 0, 3 * 4);
    memset(trans, 0, 3 * 4);
    trans[2] = -4.0;

    flag = 0;
    loaddata();

    time_in_seconds = (double) (clock() - x) / CLOCKS_PER_SEC;
    printf("Loading data %f\n", time_in_seconds);
    x = clock();

    loadlist();

    time_in_seconds = (double) (clock() - x) / CLOCKS_PER_SEC;
    printf("Creating list %f\n", time_in_seconds);
    x = clock();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(1024, 1024);
    glutInitWindowPosition(0, 0);
    glutCreateWindow(argv[0]);
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutReshapeFunc(reshape);
    glutMainLoop();
    return 0;
}


void Draw(NODE *list, int i, int j) {
    int hi = 1, wi = 1;
    glViewport(j * g_w / wi, i * g_h / hi, 0.8 * g_w / wi, 0.8 * g_h / hi);
    glPushMatrix();
    glTranslatef(-0.5, -0.5, -0.5);
    DrawIsoSurface(list);
    glPopMatrix();
    glutWireCube(1.0);
}

long diff;
clock_t start, end;
int counter = 0;

void display(void) {
    x = clock();
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glRotatef(angle, 1.0, 1.0, 1.0);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, material);
    Draw(list1, 0, 0);
    glutSwapBuffers();
    time_in_seconds = (double) (clock() - x) / CLOCKS_PER_SEC;
    printf("Display %f\n", time_in_seconds);
    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {

    switch (key) {
        case 27:
            clearlist();
            exit(0);
            break;
        case 'a':
            isoval = isoval + 0.02;
            printf("\niso value %f\n", isoval);
            x = clock();
            clearlist();
            time_in_seconds = (double) (clock() - x) / CLOCKS_PER_SEC;
            printf("Clearing List %f\n", time_in_seconds);
            x = clock();
            loadlist();
            time_in_seconds = (double) (clock() - x) / CLOCKS_PER_SEC;
            printf("Creating list1 %f\n", time_in_seconds);

            glutPostRedisplay();
            break;
        case 's':
            isoval = isoval - 0.02;
            printf("\niso value %f\n", isoval);
            clearlist();
            time_in_seconds = (double) (clock() - x) / CLOCKS_PER_SEC;
            printf("Clearing List %f\n", time_in_seconds);
            x = clock();
            loadlist();
            time_in_seconds = (double) (clock() - x) / CLOCKS_PER_SEC;
            printf("Interactivity%f\n", time_in_seconds);
            glutPostRedisplay();
            break;
        case 'l':
            angle = angle + 1;
            break;
        case 'k':
            angle = angle - 1;
            break;

    }
}


void reshape(int w, int h) {
    g_w = w;
    g_h = h;
    glViewport(0, 0, (GLsizei) w, (GLsizei) h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-0.75, 0.75, -0.75, 0.75, -10, 10);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

}

void init(void) {
    GLfloat material[] = {0.0, 0, 0.5};
    static GLfloat light_position0[] = {1.0, 1.0, 1.0, 1.0};
    static GLfloat light_ambient[] = {0.0, 0.0, 1.0, 0.0};
    static GLfloat light_position1[] = {1.0, 0.0, 2.0, 1.0};
    static GLfloat light_specular[] = {0.0, 0.0, 1.0, 0.0};
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClearDepth(1.0f);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    SetMaterial(material);

    glShadeModel(GL_SMOOTH);

    glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_DEPTH_TEST);
}

float ***Load(const char *fileName, float ***chargeDensity) {
    int index = 0, i, j, k;
    float volume, latticeConstant, chargeDen, kPoints[3];
    FILE *fp;
    fp = fopen(fileName, "r");
    if (fp == NULL) {
        printf("Error %d \n", errno);
    }
    //printf("Reading a chunk from the charge density file: %s\n", fileName);
    fscanf(fp, "%f", &latticeConstant);
    volume = latticeConstant * latticeConstant * latticeConstant;
    for (j = 0; j < 3; j++)
        fscanf(fp, "%f", &kPoints[j]);
    volume = latticeConstant * latticeConstant * latticeConstant * kPoints[0] * kPoints[0] * kPoints[0];
    fscanf(fp, "%d %d %d", &nx, &ny, &nz);
    //width = gridX; height = gridY; depth = gridZ;
    chargeDensity = (float ***) malloc(sizeof(float **) * nx);
    for (i = 0; i < nx; i++) {
        chargeDensity[i] = (float **) malloc(sizeof(float *) * ny);
        for (j = 0; j < ny; j++) {
            chargeDensity[i][j] = (float *) malloc(sizeof(float) * nz);
        }
    }
    for (i = 0; i < nz; i++)
        for (j = 0; j < ny; j++)
            for (k = 0; k < nx; k++) {
                fscanf(fp, "%f", &chargeDensity[k][j][i]);
                chargeDensity[k][j][i] /= volume;
            }
    fclose(fp);
    return (chargeDensity);
}

void freedata(float ***rho) {
    int i, j;
    for (i = 0; i < nx; i++) {
        for (j = 0; j < ny; j++)
            free(rho[i][j]);
        free(rho[i]);
    }
    free(rho);
}

void callfree() {
    freedata(data1);
    data1 = NULL;
}


void loaddata() {
    data1 = Load(file[0], data1);
}

void loadlist() {
    list1 = MarchingCube(data1, isoval, nx, ny, nz, list1);
}

void clearlist() {
    list1 = DeleteList(list1);
}

