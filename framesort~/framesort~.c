#include "ext.h"
#include "ext_sysmem.h"
#include "z_dsp.h"
#include "ext_obex.h"
#include <assert.h>
#include <stdlib.h>

typedef struct  _data {
    t_sample val;
    long idx;
} t_data;


typedef struct _framesort{
    t_pxobject w_obj;
    t_data* buffer;
    long vectorsize;
} t_framesort;

void *framesort_new(t_symbol *s,  long argc, t_atom *argv);
void framesort_free(t_framesort *x);
void framesort_perform64(t_framesort *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);
void framesort_perform64_zero(t_framesort *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);
void framesort_dsp64(t_framesort *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void framesort_assist(t_framesort *x, void *b, long m, long a, char *s);

static t_class *s_framesort_class;

void ext_main(void *r) {
    t_class *c = class_new("framesort~", (method)framesort_new, (method)framesort_free, sizeof(t_framesort), NULL, A_GIMME, 0);

    class_addmethod(c, (method)framesort_dsp64,        "dsp64",    A_CANT, 0);
    class_addmethod(c, (method)framesort_assist,    "assist",    A_CANT, 0);

    class_dspinit(c);
    class_register(CLASS_BOX, c);
    s_framesort_class = c;
}


void *framesort_new(t_symbol *s,  long argc, t_atom *argv) {
    t_framesort *x = (t_framesort *)object_alloc(s_framesort_class);
    dsp_setup((t_pxobject *)x, 1);
    outlet_new((t_object *)x, "signal");
    outlet_new((t_object *)x, "signal");

    x->vectorsize = 0;

    return (x);
}


void framesort_free(t_framesort *x) {
    sysmem_freeptr(x->buffer);
    dsp_free((t_pxobject *)x);
}


int compare_data(const void* a, const void* b) {
    t_data *a_data = (t_data*)a;
    t_data *b_data = (t_data*)b;

    if(a_data->val > b_data->val)
        return -1;
    if(a_data->val < b_data->val)
        return 1;

    return 0;
}

/* int compare_double(const void* a, const void* b) {
    if (*(double*)a > *(double*)b)
        return -1;
    if (*(double*)a < *(double*)b)
        return 1;
    return 0;
} */


void framesort_perform64(t_framesort *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam) {
    t_sample* in  = ins [0];
    t_sample* val = outs[0];
    t_sample* idx = outs[1];

    assert(sampleframes == x->vectorsize && "vectorsize mismatch!");
    for(long i=0;i<x->vectorsize;i++) {
        t_sample value = in[i];
        x->buffer[i] = (t_data){ .idx = i, .val = value };
    }

    qsort(x->buffer, x->vectorsize, sizeof(t_data), compare_data);

    // if we didn't need the indices, we could have just returned, 
    // because in and out point to the same memory

    for(long i=0;i<sampleframes;i++) {
        val[i] = x->buffer[i].val;
        idx[i] = x->buffer[i].idx;
    }

}

void framesort_perform64_zero(t_framesort *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam) {
    for(int i=0;i<numouts;i++)
        set_zero64(outs[i], sampleframes);
}

void framesort_dsp64(t_framesort *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags) {

    if(count[0]) {
        if(maxvectorsize > x->vectorsize) {
            x->vectorsize = maxvectorsize;
            sysmem_freeptr(x->buffer);
            x->buffer = (t_data*)sysmem_newptr(x->vectorsize * sizeof(t_data));
        }
        object_method(dsp64, gensym("dsp_add64"), x, framesort_perform64, 0, NULL);
    } else {
        object_method(dsp64, gensym("dsp_add64"), x, framesort_perform64_zero, 0, NULL);
    }

}


void framesort_assist(t_framesort *x, void *b, long m, long a, char *s) {
    if (m == ASSIST_INLET) {
        switch (a) {
            case 0: snprintf_zero(s, 256, "Signal in");    break;
        }
    } else{
        switch (a) {
            case 0: snprintf_zero(s, 256, "Sorted out (largest first)");    break;
            case 1: snprintf_zero(s, 256, "Indices");    break;
        }
    }
}

