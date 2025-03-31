#include "ext.h"
#include "z_dsp.h"
#include "ext_obex.h"
#include "float.h"

typedef struct _framenth{
	t_pxobject w_obj;
} t_framenth;

void *framenth_new(t_symbol *s,  long argc, t_atom *argv);
void framenth_free(t_framenth *x);
void framenth_perform64(t_framenth *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);
void framenth_perform64_zero(t_framenth *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);
void framenth_dsp64(t_framenth *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void framenth_assist(t_framenth *x, void *b, long m, long a, char *s);

static t_class *s_framenth_class;

void ext_main(void *r) {
	t_class *c = class_new("framenth~", (method)framenth_new, (method)framenth_free, sizeof(t_framenth), NULL, A_GIMME, 0);

	class_addmethod(c, (method)framenth_dsp64,		"dsp64",	A_CANT, 0);
	class_addmethod(c, (method)framenth_assist,		"assist",	A_CANT, 0);

	class_dspinit(c);
	class_register(CLASS_BOX, c);
	s_framenth_class = c;
}


void *framenth_new(t_symbol *s,  long argc, t_atom *argv) {
	t_framenth *x = (t_framenth *)object_alloc(s_framenth_class);
	dsp_setup((t_pxobject *)x, 2);
    outlet_new((t_object *)x, "signal");

	return (x);
}


void framenth_free(t_framenth *x) {
	dsp_free((t_pxobject *)x);
}


void framenth_perform64(t_framenth *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam) {
    t_sample* input  = ins [0];
    t_sample* index  = ins [1];
    t_sample* output = outs[0];

    long n = sampleframes;
    while(--n) {
        long idx = (long)(*index++);

        idx = CLAMP(idx, 0, sampleframes-1);
        *output++ = input[idx];

    }

}

void framenth_perform64_zero(t_framenth *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam) {
    for(int i=0;i<numouts;i++)
        set_zero64(outs[i], sampleframes);
}

void framenth_dsp64(t_framenth *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags) {
    if(count[0] && count[1])
        object_method(dsp64, gensym("dsp_add64"), x, framenth_perform64, 0, NULL);
    else
        object_method(dsp64, gensym("dsp_add64"), x, framenth_perform64_zero, 0, NULL);

}


void framenth_assist(t_framenth *x, void *b, long m, long a, char *s) {
	if (m == ASSIST_INLET) {
		switch (a) {
			case 0:	snprintf_zero(s, 256, "Signal to sample");  break;
			case 1:	snprintf_zero(s, 256, "Index in vector");	break;
		}
	} else{
		switch (a) {
			case 0:	snprintf_zero(s, 256, "Value at index");	break;
		}
	}
}

