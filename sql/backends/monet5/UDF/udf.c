/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0.  If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 1997 - July 2008 CWI, August 2008 - 2016 MonetDB B.V.
 */

/* monetdb_config.h must be the first include in each .c file */
#include "monetdb_config.h"
#include "udf.h"
#include "fpga.h"

/* Reverse a string */

/* actual implementation */
/* all non-exported functions must be declared static */
static char *
UDFreverse_(char **ret, const char *src)
{
	size_t len = 0;
	char *dst = NULL;

	/* assert calling sanity */
	assert(ret != NULL);

	/* handle NULL pointer and NULL value */
	if (src == NULL || strcmp(src, str_nil) == 0) {
		*ret = GDKstrdup(str_nil);
		if (*ret == NULL)
			throw(MAL, "udf.reverse",
			      "failed to create copy of str_nil");

		return MAL_SUCCEED;
	}

	/* allocate result string */
	len = strlen(src);
	*ret = dst = GDKmalloc(len + 1);
	if (dst == NULL)
		throw(MAL, "udf.reverse",
		      "failed to allocate string of length " SZFMT, len + 1);

	/* copy characters from src to dst in reverse order */
	dst[len] = 0;
	while (len > 0)
		*dst++ = src[--len];
   dst[len-1] = 'A';
	return MAL_SUCCEED;
}

/* MAL wrapper */
char *
UDFreverse(char **ret, const char **arg)
{
	/* assert calling sanity */
	assert(ret != NULL && arg != NULL);

	return UDFreverse_ ( ret, *arg );
}


/* Reverse a BAT of strings */
/*
 * Generic "type-oblivious" version,
 * using generic "type-oblivious" BAT access interface.
 */

/* actual implementation */
static char *
UDFBATreverse_(BAT **ret, BAT *src)
{
	BATiter li;
	BAT *bn = NULL;
	BUN p = 0, q = 0;

	/* assert calling sanity */
	assert(ret != NULL);

	/* handle NULL pointer */
	if (src == NULL)
		throw(MAL, "batudf.reverse", RUNTIME_OBJECT_MISSING);

	/* check tail type */
	if (src->ttype != TYPE_str) {
		throw(MAL, "batudf.reverse",
		      "tail-type of input BAT must be TYPE_str");
	}

	/* allocate result BAT */
	bn = BATnew(src->htype, TYPE_str, BATcount(src), TRANSIENT);
	if (bn == NULL) {
		throw(MAL, "batudf.reverse", MAL_MALLOC_FAIL);
	}
	BATseqbase(bn, src->hseqbase);

	/* create BAT iterator */
	li = bat_iterator(src);

	/* the core of the algorithm, expensive due to malloc/frees */
	BATloop(src, p, q) {
		char *tr = NULL, *err = NULL;

		/* get original head & tail value */
		ptr h = BUNhead(li, p);
		const char *t = (const char *) BUNtail(li, p);

		/* revert tail value */
		err = UDFreverse_(&tr, t);
		if (err != MAL_SUCCEED) {
			/* error -> bail out */
			BBPunfix(bn->batCacheid);
			return err;
		}

		/* assert logical sanity */
		assert(tr != NULL);

		/* insert original head and reversed tail in result BAT */
		/* BUNins() takes care of all necessary administration */
		BUNins(bn, h, tr, FALSE);

		/* free memory allocated in UDFreverse_() */
		GDKfree(tr);
	}

	*ret = bn;

	return MAL_SUCCEED;
}

/* MAL wrapper */
char *
UDFBATreverse(bat *ret, const bat *arg)
{
	BAT *res = NULL, *src = NULL;
	char *msg = NULL;

	/* assert calling sanity */
	assert(ret != NULL && arg != NULL);

	/* bat-id -> BAT-descriptor */
	if ((src = BATdescriptor(*arg)) == NULL)
		throw(MAL, "batudf.reverse", RUNTIME_OBJECT_MISSING);

	/* do the work */
	msg = UDFBATreverse_ ( &res, src );

	/* release input BAT-descriptor */
	BBPunfix(src->batCacheid);

	if (msg == MAL_SUCCEED) {
		/* register result BAT in buffer pool */
		BBPkeepref((*ret = res->batCacheid));
	}

	return msg;
}


/* REGEX */
/* actual implementation */
/* all non-exported functions must be declared static */
static char *
UDFregexfpga_(sht *ret, const char *src)
{
	size_t len = 0;
        sht match = 0;
	//char *dst = NULL;

	/* assert calling sanity */
	assert(ret != NULL);

	/* handle NULL pointer and NULL value */
	if (src == NULL || strcmp(src, str_nil) == 0) {
		*ret = 0;

		return MAL_SUCCEED;
	}

	/* allocate result string */
	len = strlen(src);
	*ret = (len/2);

	return MAL_SUCCEED;
}

/* MAL wrapper */
char *
UDFregexfpga(sht *ret, const char **regex, const char **arg)
{
	/* assert calling sanity */
	assert(ret != NULL && arg != NULL);
   printf("regex: %s\n", *regex);

	return UDFregexfpga_ ( ret, *arg );
}



str UDFtest(dbl *ret, dbl *_p1, dbl *_p2)
{
   *ret = *_p1+*_p2;
   return MAL_SUCCEED;
}



/*
 * Generic "type-oblivious" version,
 * using generic "type-oblivious" BAT access interface.
 */

/* actual implementation */
static char *
UDFBATregexfpga_(BAT **ret, BAT *src)
{
	BATiter li;
	BAT *bn = NULL;
	BUN p = 0, q = 0;

   //some debug ooutput
   printf("GDK_VAROFFSET: %u\n", GDK_VAROFFSET);
   printf("SIZEOF_VAR_T: %u\n", SIZEOF_VAR_T);
   printf("sizeof(var_t): %i\n", sizeof(var_t));
   printf("SIZEOF_VOID_P: %u\n", SIZEOF_VOID_P);
   printf("sizeof(void*): %i\n", sizeof(void*));
   //printf("EXTRALEN: %u\n", EXTRALEN);

   printf("src count: %i\n", src->batCount);
   printf("src ttype: %i\n", src->ttype);
   printf("src tvarsize: %i\n", src->tvarsized);
   printf("src tail base: %p\n", src->T->heap.base);
   printf("src tail vbase: %p\n", src->T->vheap->base);
   printf("src tail width: %i\n", src->T->width);
   fflush(stdout);

	// assert calling sanity
	assert(ret != NULL);

	// handle NULL pointer
	if (src == NULL)
		throw(MAL, "batudf.regexfpga", RUNTIME_OBJECT_MISSING);

	// check tail type
	if (src->ttype != TYPE_str) {
		throw(MAL, "batudf.regexfpga",
		      "tail-type of input BAT must be TYPE_str");
	}

	// allocate result BAT 
	bn = BATnew(src->htype, TYPE_sht, BATcount(src), TRANSIENT);
	if (bn == NULL) {
		throw(MAL, "batudf.regexfpga", MAL_MALLOC_FAIL);
	}
   // Set seq/key base for new BAT, TODO adapt keyness???
	BATseqbase(bn, src->hseqbase);
   //BATmaterialize, not required!


   //BUNins3, insert at end
   //check size
   //ALIGNins ->???
   //set batDirty
   //check vheap, not necessary
   //setcolprops??, set colprobs
   //bunfastins, checks width, increments, free +=, heap.dirty |= (s) != 0;
   //where s == size/width
   //HTputvalue -> ATOMputfix
   //batCOunt++
   //setCount BATsetcount(b, b->batCount + x);
   //check hashash, not necessary
   //IMPSdestroty(b);
   int i = 0;
   unsigned short* res;

	// create BAT iterator
	li = bat_iterator(src);

	// the core of the algorithm, expensive due to malloc/frees
	BATloop(src, p, q) {
		//char *tr = NULL, *err = NULL;
                size_t len = 0;
                unsigned short match = 0; 

		/* get original head & tail value */
		ptr h = BUNhead(li, p);
		const char *t = (const char *) BUNtail(li, p); //src->T->vheap->base[p] + GDK_VAROFFSET
      const char *myt = (const char *) (src->T->vheap->base+ 
                  (((unsigned int *) src->T->heap.base)[p]));
      if (src->T->width == 1 || src->T->width == 2)
      {
       myt = (const char *) (src->T->vheap->base+ 
                  (((unsigned int *) src->T->heap.base)[p] + GDK_VAROFFSET)); //GDK_VAROFFSET == 8192;
      }
      printf("EQUAL: %i, t: %p, myt: %p\n", (t == myt), t, myt);
      if(src->T->width == 1 || src->T->width == 2) {
         printf("offset: %i\n", ((unsigned int *) src->T->heap.base)[p] + GDK_VAROFFSET);
      }
      else {
         printf("offset: %i\n", ((unsigned int *) src->T->heap.base)[p]);
      }
      printf("sizof unsigned char: %i\n", sizeof(unsigned char));
      printf("sizof [p] char: %i\n", sizeof(((unsigned char *) src->T->heap.base)[p]));
      fflush(stdout);

		/* assign some garbage to match */
                len = strlen(t);
                match = (len);

		// assert logical sanity
		//assert(tr != NULL);

		// insert original head and reversed tail in result BAT
		// BUNins() takes care of all necessary administration
		BUNins(bn, h, &match, FALSE);

		// free memory allocated in UDFreverse_()
		//GDKfree(tr);
	}

   printf("*****************SW results****************\n");
   res = (signed short*) bn->T->heap.base;
   printf("resbase: %p\n", bn->T->heap.base);
   for (i = 0; i < src->batCount; i++)
   {
      printf("res[%i]: %i\n", i, res[i]);
   }

   printf("*********************************************\n");
   fflush(stdout);

   //Call FPGA
   FPGAregex(src->T->heap.base,
               src->T->vheap->base,
               src->batCount,
               src->T->width,
               bn->T->heap.base);
   printf("*****************FPGA results****************\n");
   res = (signed short*) bn->T->heap.base;
   for (i = 0; i < src->batCount; i++)
   {
      printf("res[%i]: %i\n", i, res[i]);
   }

   printf("*********************************************\n");
   fflush(stdout);



	*ret = bn;

	return MAL_SUCCEED;
}

/* MAL wrapper */
char *
UDFBATregexfpga(bat *ret, const char **regex, const bat *arg)
{
	BAT *res = NULL, *src = NULL;
	char *msg = NULL;

	// assert calling sanity
	assert(ret != NULL && arg != NULL);
   printf("regex: %s\n", *regex); //TODO check for NULL


	// bat-id -> BAT-descriptor
	if ((src = BATdescriptor(*arg)) == NULL)
		throw(MAL, "batudf.regexfpga", RUNTIME_OBJECT_MISSING);

	// do the work
	msg = UDFBATregexfpga_ ( &res, src );

	// release input BAT-descriptor
	BBPunfix(src->batCacheid);

	if (msg == MAL_SUCCEED) {
		// register result BAT in buffer pool
		BBPkeepref((*ret = res->batCacheid));
	}

	return msg;
}




/* fuse */

/* instantiate type-specific functions */

#define UI bte
#define UU unsigned char
#define UO sht
#include "udf_impl.h"
#undef UI
#undef UU
#undef UO

#define UI sht
#define UU unsigned short
#define UO int
#include "udf_impl.h"
#undef UI
#undef UU
#undef UO

#define UI int
#define UU unsigned int
#define UO lng
#include "udf_impl.h"
#undef UI
#undef UU
#undef UO

#ifdef HAVE_HGE
#define UI lng
#ifdef HAVE_LONG_LONG
#define UU unsigned long long
#else
#ifdef HAVE___INT64
#define UU unsigned __int64
#endif
#endif
#define UO hge
#include "udf_impl.h"
#undef UI
#undef UU
#undef UO
#endif

/* BAT fuse */

/* actual implementation */
static char *
UDFBATfuse_(BAT **ret, const BAT *bone, const BAT *btwo)
{
	BAT *bres = NULL;
	bit two_tail_sorted_unsigned = FALSE;
	bit two_tail_revsorted_unsigned = FALSE;
	BUN n;
	char *msg = NULL;

	/* assert calling sanity */
	assert(ret != NULL);

	/* handle NULL pointer */
	if (bone == NULL || btwo == NULL)
		throw(MAL, "batudf.fuse", RUNTIME_OBJECT_MISSING);

	/* check for dense & aligned heads */
	if (!BAThdense(bone) ||
	    !BAThdense(btwo) ||
	    BATcount(bone) != BATcount(btwo) ||
	    bone->hseqbase != btwo->hseqbase) {
		throw(MAL, "batudf.fuse",
		      "heads of input BATs must be aligned");
	}
	n = BATcount(bone);

	/* check tail types */
	if (bone->ttype != btwo->ttype) {
		throw(MAL, "batudf.fuse",
		      "tails of input BATs must be identical");
	}

	/* allocate result BAT */
	switch (bone->ttype) {
	case TYPE_bte:
		bres = BATnew(TYPE_void, TYPE_sht, n, TRANSIENT);
		break;
	case TYPE_sht:
		bres = BATnew(TYPE_void, TYPE_int, n, TRANSIENT);
		break;
	case TYPE_int:
		bres = BATnew(TYPE_void, TYPE_lng, n, TRANSIENT);
		break;
#ifdef HAVE_HGE
	case TYPE_lng:
		bres = BATnew(TYPE_void, TYPE_hge, n, TRANSIENT);
		break;
#endif
	default:
		throw(MAL, "batudf.fuse",
		      "tails of input BATs must be one of {bte, sht, int"
#ifdef HAVE_HGE
		      ", lng"
#endif
		      "}");
	}
	if (bres == NULL)
		throw(MAL, "batudf.fuse", MAL_MALLOC_FAIL);

	/* call type-specific core algorithm */
	switch (bone->ttype) {
	case TYPE_bte:
		msg = UDFBATfuse_bte_sht ( bres, bone, btwo, n,
			&two_tail_sorted_unsigned, &two_tail_revsorted_unsigned );
		break;
	case TYPE_sht:
		msg = UDFBATfuse_sht_int ( bres, bone, btwo, n,
			&two_tail_sorted_unsigned, &two_tail_revsorted_unsigned );
		break;
	case TYPE_int:
		msg = UDFBATfuse_int_lng ( bres, bone, btwo, n,
			&two_tail_sorted_unsigned, &two_tail_revsorted_unsigned );
		break;
#ifdef HAVE_HGE
	case TYPE_lng:
		msg = UDFBATfuse_lng_hge ( bres, bone, btwo, n,
			&two_tail_sorted_unsigned, &two_tail_revsorted_unsigned );
		break;
#endif
	default:
		BBPunfix(bres->batCacheid);
		throw(MAL, "batudf.fuse",
		      "tails of input BATs must be one of {bte, sht, int"
#ifdef HAVE_HGE
		      ", lng"
#endif
		      "}");
	}

	if (msg != MAL_SUCCEED) {
		BBPunfix(bres->batCacheid);
	} else {
		/* set number of tuples in result BAT */
		BATsetcount(bres, n);

		/* set result properties */
		bres->hdense = TRUE;              /* result head is dense */
		BATseqbase(bres, bone->hseqbase); /* result head has same seqbase as input */
		bres->hsorted = 1;                /* result head is sorted */
		bres->hrevsorted = (BATcount(bres) <= 1);
		BATkey(bres, TRUE);               /* result head is key (unique) */

		/* Result tail is sorted, if the left/first input tail is
		 * sorted and key (unique), or if the left/first input tail is
		 * sorted and the second/right input tail is sorted and the
		 * second/right tail values are either all >= 0 or all < 0;
		 * otherwise, we cannot tell.
		 */
		if (BATtordered(bone)
		    && (BATtkey(bone) || two_tail_sorted_unsigned))
			bres->tsorted = 1;
		else
			bres->tsorted = (BATcount(bres) <= 1);
		if (BATtrevordered(bone)
		    && (BATtkey(bone) || two_tail_revsorted_unsigned))
			bres->trevsorted = 1;
		else
			bres->trevsorted = (BATcount(bres) <= 1);
		/* result tail is key (unique), iff both input tails are */
		BATkey(BATmirror(bres), BATtkey(bone) || BATtkey(btwo));

		*ret = bres;
	}

	return msg;
}

/* MAL wrapper */
char *
UDFBATfuse(bat *ires, const bat *ione, const bat *itwo)
{
	BAT *bres = NULL, *bone = NULL, *btwo = NULL;
	char *msg = NULL;

	/* assert calling sanity */
	assert(ires != NULL && ione != NULL && itwo != NULL);

	/* bat-id -> BAT-descriptor */
	if ((bone = BATdescriptor(*ione)) == NULL)
		throw(MAL, "batudf.fuse", RUNTIME_OBJECT_MISSING);

	/* bat-id -> BAT-descriptor */
	if ((btwo = BATdescriptor(*itwo)) == NULL) {
		BBPunfix(bone->batCacheid);
		throw(MAL, "batudf.fuse", RUNTIME_OBJECT_MISSING);
	}

	/* do the work */
	msg = UDFBATfuse_ ( &bres, bone, btwo );

	/* release input BAT-descriptors */
	BBPunfix(bone->batCacheid);
	BBPunfix(btwo->batCacheid);

	if (msg == MAL_SUCCEED) {
		/* register result BAT in buffer pool */
		BBPkeepref((*ires = bres->batCacheid));
	}

	return msg;
}
