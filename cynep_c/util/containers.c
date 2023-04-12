/* 
DOCUMENTATION

  Dynamic Arrays

    Non-function interface:

      Declare an empty dynamic array of type T
        T* foo = NULL;

      Access the i'th item of a dynamic array 'foo' of type T, T* foo:
        foo[i]

    Functions (actually macros)

      arrfree:
        void arrfree(T*);
          Frees the array.

      arrlen:
        ptrdiff_t arrlen(T*);
          Returns the number of elements in the array.

      arrlenu:
        size_t arrlenu(T*);
          Returns the number of elements in the array as an unsigned type.

      arrpop:
        T arrpop(T* a)
          Removes the final element of the array and returns it.

      arrput:
        T arrput(T* a, T b);
          Appends the item b to the end of array a. Returns b.

      arrins:
        T arrins(T* a, int p, T b);
          Inserts the item b into the middle of array a, into a[p],
          moving the rest of the array over. Returns b.

      arrinsn:
        void arrinsn(T* a, int p, int n);
          Inserts n uninitialized items into array a starting at a[p],
          moving the rest of the array over.

      arraddnptr:
        T* arraddnptr(T* a, int n)
          Appends n uninitialized items onto array at the end.
          Returns a pointer to the first uninitialized item added.

      arraddnindex:
        size_t arraddnindex(T* a, int n)
          Appends n uninitialized items onto array at the end.
          Returns the index of the first uninitialized item added.

      arrdel:
        void arrdel(T* a, int p);
          Deletes the element at a[p], moving the rest of the array over.

      arrdeln:
        void arrdeln(T* a, int p, int n);
          Deletes n elements starting at a[p], moving the rest of the array over.

      arrdelswap:
        void arrdelswap(T* a, int p);
          Deletes the element at a[p], replacing it with the element from
          the end of the array. O(1) performance.

      arrsetlen:
        void arrsetlen(T* a, int n);
          Changes the length of the array to n. Allocates uninitialized
          slots at the end if necessary.

      arrsetcap:
        size_t arrsetcap(T* a, int n);
          Sets the length of allocated storage to at least n. It will not
          change the length of the array.

      arrcap:
        size_t arrcap(T* a);
          Returns the number of total elements the array can contain without
          needing to be reallocated.
*/

#pragma once

#define array_length    stbds_arrlen
#define arrlenu         stbds_arrlenu
#define array_push      stbds_arrput
#define arrpop          stbds_arrpop
#define arrfree         stbds_arrfree
#define arraddnptr      stbds_arraddnptr
#define arraddnindex    stbds_arraddnindex
#define arrsetlen       stbds_arrsetlen
#define arrlast         stbds_arrlast
#define arrins          stbds_arrins
#define arrinsn         stbds_arrinsn
#define arrdel          stbds_arrdel
#define arrdeln         stbds_arrdeln
#define arrdelswap      stbds_arrdelswap
#define arrcap          stbds_arrcap
#define arrsetcap       stbds_arrsetcap

extern void* array_grow_f(void *a, size_t elemsize, size_t addlen, size_t min_cap);
extern void  array_free_f(void *a);

#define stbds_header(t)         ((ArrayHeader *) (t) - 1)
#define stbds_arrsetcap(a,n)    (stbds_arrgrow(a,0,n))
#define stbds_arrsetlen(a,n)    ((stbds_arrcap(a) < (size_t) (n) ? stbds_arrsetcap((a),(size_t)(n)),0 : 0), (a) ? stbds_header(a)->length = (size_t) (n) : 0)
#define stbds_arrcap(a)         ((a) ? stbds_header(a)->capacity : 0)
#define stbds_arrlen(a)         ((a) ? (ptrdiff_t) stbds_header(a)->length : 0)
#define stbds_arrlenu(a)        ((a) ?             stbds_header(a)->length : 0)
#define stbds_arrput(a,v)       (stbds_arrmaybegrow(a,1), (a)[stbds_header(a)->length++] = (v))
#define stbds_arrpop(a)         (stbds_header(a)->length--, (a)[stbds_header(a)->length])
#define stbds_arraddnptr(a,n)   (stbds_arrmaybegrow(a,n), (n) ? (stbds_header(a)->length += (n), &(a)[stbds_header(a)->length-(n)]) : (a))
#define stbds_arraddnindex(a,n) (stbds_arrmaybegrow(a,n), (n) ? (stbds_header(a)->length += (n), stbds_header(a)->length-(n)) : stbds_arrlen(a))
#define stbds_arraddnoff        stbds_arraddnindex
#define stbds_arrlast(a)        ((a)[stbds_header(a)->length-1])
#define stbds_arrfree(a)        ((void) ((a) ? array_free_f(a) : (void)0), (a)=NULL)
#define stbds_arrdel(a,i)       stbds_arrdeln(a,i,1)
#define stbds_arrdeln(a,i,n)    (memmove(&(a)[i], &(a)[(i)+(n)], sizeof *(a) * (stbds_header(a)->length-(n)-(i))), stbds_header(a)->length -= (n))
#define stbds_arrdelswap(a,i)   ((a)[i] = stbds_arrlast(a), stbds_header(a)->length -= 1)
#define stbds_arrins(a,i,v)     (stbds_arrinsn((a),(i),1), (a)[i]=(v))
#define stbds_arrmaybegrow(a,n) ((!(a) || stbds_header(a)->length + (n) > stbds_header(a)->capacity) ? (stbds_arrgrow(a,n,0),0) : 0)
#define stbds_arrgrow(a,b,c)    ((a) = array_grow_f((a), sizeof *(a), (b), (c)))

typedef struct
{
  size_t      length;
  size_t      capacity;
} ArrayHeader;

void* array_grow_f(void *a, size_t elemsize, size_t addlen, size_t min_cap)
{
  void *b;
  size_t min_len = stbds_arrlen(a) + addlen;

  // compute the minimum capacity needed
  if (min_len > min_cap)
    min_cap = min_len;

  if (min_cap <= stbds_arrcap(a))
    return a;

  // increase needed capacity to guarantee O(1) amortized
  if (min_cap < 2 * stbds_arrcap(a))
    min_cap = 2 * stbds_arrcap(a);
  else if (min_cap < 4)
    min_cap = 4;

  b = realloc((a) ? stbds_header(a) : 0, elemsize * min_cap + sizeof(ArrayHeader));

  b = (char *) b + sizeof(ArrayHeader);

  if (a == NULL) {
    stbds_header(b)->length = 0;
  }

  stbds_header(b)->capacity = min_cap;

  return b;
}

void array_free_f(void *a)
{
  free(stbds_header(a));
}