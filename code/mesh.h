#include <vector>
#include <cassert>
#include <fstream>
#include <map>
#include <cmath>
#include <ctime>
#include <unordered_map>

using namespace std;

int hid_counter = 0;
int tid_counter = 0;
int sid_counter = 0;

float frandom()
{
    float r = (float)rand()/(float)RAND_MAX;
    return r;
}


struct HalfEdge
{
    int id;
    int n;//next
    int o;//opposite
    int t;//triangle
    int s0,s1;//sommet
};

HalfEdge make_halfedge(int n=-1,int o=-1,int t=-1,int s0=-1,int s1=-1)
{
    HalfEdge h;
    h.id = hid_counter++;
    h.n = n;
    h.o = o;
    h.t = t;
    h.s0 = s0;
    h.s1 = s1;
    return h;
}

struct Triangle
{
    int id;
    int e0,e1,e2;
};

Triangle make_triangle(int e0=-1,int e1=-1,int e2=-1)
{
    Triangle t;
    t.id = tid_counter++;
    t.e0 = e0;
    t.e1 = e1;
    t.e2 = e2;
    return t;
}

struct Sommet
{
    int id;
    float x,y,z;
//    int he;
};

Sommet make_sommet(float x=0,float y=0,float z=0,int he=-1)
{
    Sommet s;
    s.id = sid_counter++;
    s.x = x;
    s.y = y;
    s.z = z;
    //s.he = he;
    return s;   
}



struct Maillage
{
    std::vector<Sommet> sommets;
    std::vector<Triangle> triangles;
    std::vector<HalfEdge> halfedges;
    int triangulated;
    
    void add_sommet(Sommet s)
    {
        sommets.push_back(s);
    }
    Sommet& get_sommet(int id)
    {
        return sommets[id];
    }

    void add_triangle(Triangle t)
    {
        triangles.push_back(t);
    }
    Triangle& get_triangle(int id)
    {
        return triangles[id];
    }

    void add_halfedge(HalfEdge h)
    {
        halfedges.push_back(h);
    }
    HalfEdge& get_halfedge(int id)
    {
        return halfedges[id];
    }
    
    void generate_sommets(int N)
    {
        float extent = 700;
//        srand(time(0));
        add_sommet(make_sommet(10,10));
        add_sommet(make_sommet(20+extent*frandom(),10));
        add_sommet(make_sommet(10,20+extent*frandom()));
        for(int i=3;i<N;i++)
        {
            Sommet s = make_sommet(extent*frandom(),extent*frandom(),extent*frandom());
            add_sommet(s);
        }
    }
    
    int sommet_voit(Sommet s,Sommet s0,Sommet s1)//sommet 's' voit arrete 's0'->'s1'
    {
        float ax0 = s0.x;
        float ay0 = s0.y;
        float ax1 = s1.x;
        float ay1 = s1.y;
        
        float x0 = ax1 - ax0;
        float y0 = ay1 - ay0;

        float x1 = s.x - ax0;
        float y1 = s.y - ay0;
        
        float cross = -(x0*y1 - y0*x1);

        return (cross<0);
    }
    
    int sommet_inside_tri(Sommet s,Triangle t)
    {
        HalfEdge e0 = get_halfedge(t.e0);
        HalfEdge e1 = get_halfedge(t.e1);
        HalfEdge e2 = get_halfedge(t.e2);
        return (!sommet_voit(s,get_sommet(e0.s0),get_sommet(e0.s1)) &&
                !sommet_voit(s,get_sommet(e1.s0),get_sommet(e1.s1)) &&
                !sommet_voit(s,get_sommet(e2.s0),get_sommet(e2.s1)));
    }

    int is_CW(Sommet s0,Sommet s1, Sommet s2)
    {
        return sommet_voit(s2,s0,s1);
    }

    
    void centre_cercle(int s0,int s1,int s2,float* xcc,float* ycc)
    {
        float xa = get_sommet(s0).x;
        float ya = get_sommet(s0).y;
        
        float xb = get_sommet(s1).x;
        float yb = get_sommet(s1).y;

        float xc = get_sommet(s2).x;
        float yc = get_sommet(s2).y;

        float D = 2*(xa*(yb-yc) + xb*(yc-ya) + xc*(ya-yb));
        
        *xcc = 1.f/D * ((xa*xa + ya*ya)*(yb-yc) +  (xb*xb + yb*yb)*(yc-ya) + (xc*xc + yc*yc)*(ya-yb));
        *ycc = 1.f/D * ((xa*xa + ya*ya)*(xc-xb) +  (xb*xb + yb*yb)*(xa-xc) + (xc*xc + yc*yc)*(xb-xa));
    }

    float D2(float x0,float y0,float x1,float y1)
    {
        return ((x1-x0)*(x1-x0) + (y1-y0)*(y1-y0));
    }
    
    int arrete_illegale(int s0,int s1,int s2,int s3)
    {
        float xc1,yc1,xc2,yc2;
        centre_cercle(s0,s1,s2,&xc1,&yc1);
//        centre_cercle(s0,s3,s1,&xc2,&yc2);

        float x0 = get_sommet(s0).x;
        float y0 = get_sommet(s0).y;

        float x1 = get_sommet(s1).x;
        float y1 = get_sommet(s1).y;

        float x2 = get_sommet(s2).x;
        float y2 = get_sommet(s2).y;

        float x3 = get_sommet(s3).x;
        float y3 = get_sommet(s3).y;

        if(D2(xc1,yc1,x0,y0) > D2(xc1,yc1,x3,y3))
            return 1;

        //if(D2(xc2,yc2,x0,y0) > D2(xc2,yc2,x2,y2))
        //    return 1;

        return 0;
    }

    
    void delaunay()
    {
        bool nothing_flipped;

        do
        {
            nothing_flipped = true;
            vector<bool> flipped(halfedges.size(),false);

            for(int i=0;i<halfedges.size();i++)
            {
                HalfEdge& he = halfedges[i];

                if(he.o==-1 || flipped[he.id]) continue; // boundary

                HalfEdge& heo = get_halfedge(he.o);
                HalfEdge& e0 = get_halfedge(heo.n);
                HalfEdge& e1 = get_halfedge(e0.n);
                HalfEdge& e2 = get_halfedge(he.n);
                HalfEdge& e3 = get_halfedge(e2.n);

                Sommet s0 = get_sommet(he.s0);
                Sommet s3 = get_sommet(e1.s0);
                Sommet s1 = get_sommet(e2.s0);
                Sommet s2 = get_sommet(e3.s0);

                Triangle& T0 = get_triangle(he.t);
                Triangle& T1 = get_triangle(heo.t);
            
                if(arrete_illegale(s0.id,s1.id,s2.id,s3.id))
                {
                    nothing_flipped = false;
                    he.s0 = s2.id;
                    he.s1 = s3.id;
            
                    heo.s0 = s3.id;
                    heo.s1 = s2.id;

                    T0.e0 = he.id;
                    T0.e1 = e1.id;
                    T0.e2 = e2.id;

                    T1.e0 = heo.id;
                    T1.e1 = e3.id;
                    T1.e2 = e0.id;

                    e1.t = T0.id;
                    e3.t = T1.id;

                    heo.n  = e3.id;
                    e3.n = e0.id;
                    e0.n = heo.id;

                    he.n  = e1.id;
                    e1.n  = e2.id;
                    e2.n  = he.id;
            
                    flipped[he.id]  = true;
                    flipped[heo.id] = true;
                }
                //if(--breakcount==0)break;

            }
        }while(!nothing_flipped);
    }

    
    void triangulation_naive()
    {
        triangulated = 1;
        triangles.clear();
        halfedges.clear();
        hid_counter = 0;
        tid_counter = 0;
        
        Sommet& s0 = get_sommet(sommets.begin()->id);
        Sommet& s1 = get_sommet((++sommets.begin())->id);
        Sommet& s2 = get_sommet((++(++sommets.begin()))->id);
        
        HalfEdge a0 = make_halfedge();
        HalfEdge b0 = make_halfedge();
        HalfEdge c0 = make_halfedge();

        Triangle first_tri = make_triangle();

        a0.n = b0.id;
        a0.o = -1;
        a0.t = first_tri.id;
        a0.s0 = s0.id;
        a0.s1 = s1.id;
        
        b0.n = c0.id;
        b0.o = -1;
        b0.t = first_tri.id;
        b0.s0 = s1.id;
        b0.s1 = s2.id;
        
        c0.n = a0.id;
        c0.o = -1;
        c0.t = first_tri.id;
        c0.s0 = s2.id;
        c0.s1 = s0.id;
        
        first_tri.e0 = a0.id;
        first_tri.e1 = b0.id;
        first_tri.e2 = c0.id;
    
        if(is_CW(s0,s1,s2))
        {
            a0.s1 = s2.id;
            
            b0.s0 = s2.id;
            b0.s1 = s1.id;
            
            c0.s0 = s1.id;
        }

        add_triangle(first_tri);
        add_halfedge(a0);
        add_halfedge(b0);
        add_halfedge(c0);

        for(int i=3;i<sommets.size();i++)
        {
            Sommet& s3 = sommets[i];
            int in_tri = -1;
            for(int j=0;j<triangles.size();j++)
            {
                if(sommet_inside_tri(s3,triangles[j]))
                {
                    in_tri = j;
                    break;
                }
            }

            if(in_tri!=-1)
            {
                Triangle& big_tri = get_triangle(in_tri);
                
                HalfEdge& e0 = get_halfedge(big_tri.e0);
                HalfEdge& e1 = get_halfedge(big_tri.e1);
                HalfEdge& e2 = get_halfedge(big_tri.e2);
                
                Sommet& s0 = get_sommet(e0.s0);
                Sommet& s1 = get_sommet(e1.s0);
                Sommet& s2 = get_sommet(e2.s0);

                HalfEdge
                    e3 = make_halfedge(),
                    e4 = make_halfedge(),
                    e5 = make_halfedge(),
                    e6 = make_halfedge(),
                    e7 = make_halfedge(),
                    e8 = make_halfedge();

                Triangle T0 = big_tri,T1 = make_triangle(),T2=make_triangle();
            
                e5.s0 = s0.id;
                e5.s1 = s3.id;
                e5.o  = e6.id;
                e5.n  = e3.id;
                e5.t  = T0.id;

                e3.s0 = s3.id;
                e3.s1 = s2.id;
                e3.o  = e4.id;
                e3.n  = e2.id;
                e3.t  = T1.id;

                e4.s0 = s2.id;
                e4.s1 = s3.id;
                e4.o  = e3.id;
                e4.n  = e8.id;
                e4.t  = T2.id;

                e8.s0 = s3.id;
                e8.s1 = s1.id;
                e8.o  = e7.id;
                e8.n  = e1.id;
                e8.t  = T2.id;

                e7.s0 = s1.id;
                e7.s1 = s3.id;
                e7.o  = e8.id;
                e7.n  = e6.id;
                e7.t  = T1.id;

                e6.s0 = s3.id;
                e6.s1 = s0.id;
                e6.o  = e5.id;
                e6.n  = e0.id;
                e6.t  = T1.id;

                e0.t  = T1.id;
                e1.t  = T2.id;
                e2.t  = T0.id;

                e0.n = e7.id;
                e1.n = e4.id;
                e2.n = e5.id;
                
                T0.e0 = e2.id;
                T0.e1 = e5.id;
                T0.e2 = e3.id;

                T1.e0 = e0.id;
                T1.e1 = e7.id;
                T1.e2 = e6.id;

                T2.e0 = e1.id;
                T2.e1 = e4.id;
                T2.e2 = e8.id;

                big_tri = T0;

                add_triangle(T1);
                add_triangle(T2);

                add_halfedge(e3);
                add_halfedge(e4);
                add_halfedge(e5);
                add_halfedge(e6);
                add_halfedge(e7);
                add_halfedge(e8);
            }
            else
            {
                vector<HalfEdge> halfedges_to_add;
                for(int j=0;j<halfedges.size();j++)
                {
                    HalfEdge& he = halfedges[j];
                
                    if(he.o!=-1) continue;// not boundary;
                    
                    Sommet& s0 = get_sommet(he.s0);
                    Sommet& s1 = get_sommet(he.s1);
                    
                    if(sommet_voit(s3,s0,s1))
                    {
                        Triangle T = make_triangle();
                        HalfEdge e0 = make_halfedge(), e1 = make_halfedge(), e2 = make_halfedge();

                        e0.s0 = s0.id;
                        e0.s1 = s3.id;
                        e0.n  = e1.id;
                        e0.t  = T.id;

                        e1.s0 = s3.id;
                        e1.s1 = s1.id;
                        e1.n  = e2.id;
                        e1.t  = T.id;

                        e2.s0 = s1.id;
                        e2.s1 = s0.id;
                        e2.n  = e0.id;
                        e2.o  = he.id;
                        e2.t  = T.id;

                        he.o  = e2.id;
                        
                        T.e0 = e0.id;
                        T.e1 = e1.id;
                        T.e2 = e2.id;
                        //find opposites, maybe use hashmap for efficiency

                        
                        for(int k=0;k<halfedges_to_add.size();k++)
                        {
                            if(halfedges_to_add[k].s0 == e0.s1 && halfedges_to_add[k].s1 == e0.s0)
                            {
                                e0.o = halfedges_to_add[k].id;
                                halfedges_to_add[k].o = e0.id;
                            }
                            if(halfedges_to_add[k].s0 == e1.s1 && halfedges_to_add[k].s1 == e1.s0)
                            {
                                e1.o = halfedges_to_add[k].id;
                                halfedges_to_add[k].o = e1.id;
                            }
                        }

                        for(int k=0;k<halfedges.size();k++)
                        {
                            if(halfedges[k].s0 == e0.s1 && halfedges[k].s1 == e0.s0)
                            {
                                e0.o = halfedges[k].id;
                                halfedges[k].o = e0.id;
                            }
                            if(halfedges[k].s0 == e1.s1 && halfedges[k].s1 == e1.s0)
                            {
                                e1.o = halfedges[k].id;
                                halfedges[k].o = e1.id;
                            }
                        }

                        //

                        add_triangle(T);
                        halfedges_to_add.push_back(e0);
                        halfedges_to_add.push_back(e1);
                        halfedges_to_add.push_back(e2);
                    }
                }

                for(int it=0;it<halfedges_to_add.size();it++)
                    add_halfedge(halfedges_to_add[it]);
            }
        }
    }
};
