#ifndef _json_stuff_h
#define _json_stuff_h

#ifdef __cplusplus
extern "C" {
#endif

#define ADD_STR(n,s,k) jobj_add_str((n),(k),cfg_getstr((s),(k)))

extern void init_jarray(int );
extern void init_jobj(int );
extern void add_jitem(int);
extern void jobj_add_str(int,char *,char *);
extern void jobj_add_int(int,char *,int);
extern void jarray_convert();
extern void clean_json();
extern void block_json();

#ifdef __cplusplus
};
#endif

#endif

