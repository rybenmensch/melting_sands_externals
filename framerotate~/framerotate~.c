#include "ext.h"
#include "ext_sysmem.h"
#include "z_dsp.h"
#include "ext_obex.h"
#include "float.h"
#include <string.h>

typedef struct _framerotate{
	t_pxobject w_obj;
} t_framerotate;

void *framerotate_new(t_symbol *s,  long argc, t_atom *argv);
void framerotate_free(t_framerotate *x);
void framerotate_perform64(t_framerotate *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);
void framerotate_perform64_nop(t_framerotate *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);
void framerotate_perform64_zero(t_framerotate *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);
void framerotate_dsp64(t_framerotate *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void framerotate_assist(t_framerotate *x, void *b, long m, long a, char *s);

static t_class *s_framerotate_class;

void ext_main(void *r) {
	t_class *c = class_new("framerotate~", (method)framerotate_new, (method)framerotate_free, sizeof(t_framerotate), NULL, A_GIMME, 0);

	class_addmethod(c, (method)framerotate_dsp64,		"dsp64",	A_CANT, 0);
	class_addmethod(c, (method)framerotate_assist,		"assist",	A_CANT, 0);

	class_dspinit(c);
	class_register(CLASS_BOX, c);
	s_framerotate_class = c;
}


void *framerotate_new(t_symbol *s,  long argc, t_atom *argv) {
	t_framerotate *x = (t_framerotate *)object_alloc(s_framerotate_class);
	dsp_setup((t_pxobject *)x, 2);
    outlet_new((t_object *)x, "signal");

	return (x);
}


void framerotate_free(t_framerotate *x) {
	dsp_free((t_pxobject *)x);
}


void framerotate_perform64(t_framerotate *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam) {
    t_sample* input  = ins [0];
    t_sample* shift  = ins [1];
    t_sample* output = outs[0];

    for(long i=0;i<sampleframes;i++) {
        long offset = (long)(*shift++);
        // overflow
        long index = (i + offset) % sampleframes;
        // underflow
        if(index<0) index += sampleframes;

        *output++ = input[index];

    }

}


void framerotate_perform64_nop(t_framerotate *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam) {
    for(int i=0;i<numouts;i++)
        memcpy(outs[i], ins[0], sampleframes*sizeof(t_sample));

}


void framerotate_perform64_zero(t_framerotate *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam) {
    for(int i=0;i<numouts;i++)
        set_zero64(outs[i], sampleframes);
}


void framerotate_dsp64(t_framerotate *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags) {
    if(count[0]) {
        if(count[1]) {
            object_method(dsp64, gensym("dsp_add64"), x, framerotate_perform64, 0, NULL);
        } else {
            object_method(dsp64, gensym("dsp_add64"), x, framerotate_perform64_nop, 0, NULL);

        }
    } else {
        object_method(dsp64, gensym("dsp_add64"), x, framerotate_perform64_zero, 0, NULL);
    }

}


void framerotate_assist(t_framerotate *x, void *b, long m, long a, char *s) {
	if (m == ASSIST_INLET) {
		switch (a) {
			case 0:	snprintf_zero(s, 256, "Signal to rotate");  break;
			case 1:	snprintf_zero(s, 256, "Amount by which to rotate");	break;
		}
	} else{
		switch (a) {
			case 0:	snprintf_zero(s, 256, "Rotated signal");	break;
		}
	}
}

