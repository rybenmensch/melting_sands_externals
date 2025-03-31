#include "ext.h"
#include "z_dsp.h"
#include "ext_obex.h"
#include "float.h"

typedef struct _frameminmax{
	t_pxobject w_obj;
} t_frameminmax;

void *frameminmax_new(t_symbol *s,  long argc, t_atom *argv);
void frameminmax_free(t_frameminmax *x);
void frameminmax_perform64(t_frameminmax *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);
void frameminmax_perform64_zero(t_frameminmax *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);
void frameminmax_dsp64(t_frameminmax *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void frameminmax_assist(t_frameminmax *x, void *b, long m, long a, char *s);

static t_class *s_frameminmax_class;

void ext_main(void *r) {
	t_class *c = class_new("frameminmax~", (method)frameminmax_new, (method)frameminmax_free, sizeof(t_frameminmax), NULL, A_GIMME, 0);

	class_addmethod(c, (method)frameminmax_dsp64,		"dsp64",	A_CANT, 0);
	class_addmethod(c, (method)frameminmax_assist,		"assist",	A_CANT, 0);

	class_dspinit(c);
	class_register(CLASS_BOX, c);
	s_frameminmax_class = c;
}


void *frameminmax_new(t_symbol *s,  long argc, t_atom *argv) {
    t_frameminmax *x = (t_frameminmax *)object_alloc(s_frameminmax_class);
    dsp_setup((t_pxobject *)x, 1);
    for(int i=0;i<4;i++)
        outlet_new((t_object *)x, "signal");

    return (x);
}


void frameminmax_free(t_frameminmax *x) {
	dsp_free((t_pxobject *)x);
}


void frameminmax_perform64(t_frameminmax *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam) {
    t_sample* in        = ins [0];

    t_sample* minValOut = outs[0];
    t_sample* minIdxOut = outs[1];
    t_sample* maxValOut = outs[2];
    t_sample* maxIdxOut = outs[3];

    t_sample minVal =  DBL_MAX;
    long minIdx = 0;
    t_sample maxVal = -DBL_MAX;
    long maxIdx = 0;

    for(long i=0;i<sampleframes;i++) {
        t_sample val = in[i];

        if(val<minVal) {
            minVal = val;
            minIdx = i;
        }

        if(val>maxVal) {
            maxVal = val;
            maxIdx = i;
        }
    }

    long n = sampleframes;
    while(--n) {
        *minValOut++ = minVal;
        *minIdxOut++ = minIdx;

        *maxValOut++ = maxVal;
        *maxIdxOut++ = maxIdx;
    }

}

void frameminmax_perform64_zero(t_frameminmax *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam) {
    for(int i=0;i<numouts;i++)
        set_zero64(outs[i], sampleframes);
}

void frameminmax_dsp64(t_frameminmax *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags) {
    if(count[0])
        object_method(dsp64, gensym("dsp_add64"), x, frameminmax_perform64, 0, NULL);
    else
        object_method(dsp64, gensym("dsp_add64"), x, frameminmax_perform64_zero, 0, NULL);

}


void frameminmax_assist(t_frameminmax *x, void *b, long m, long a, char *s) {
	if (m == ASSIST_INLET) {
		switch (a) {
			case 0:	snprintf_zero(s, 256, "Signal in");	break;
		}
	} else{
		switch (a) {
			case 0:	snprintf_zero(s, 256, "Minimum value");	break;
			case 1:	snprintf_zero(s, 256, "Minimum index");	break;
			case 2:	snprintf_zero(s, 256, "Maximum value");	break;
			case 3:	snprintf_zero(s, 256, "Maximum index");	break;
		}
	}
}

