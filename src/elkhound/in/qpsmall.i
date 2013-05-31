# 1 "qpmouse.c"



static __inline__ void atomic_sub(int i, volatile atomic_t *v)
{
	__asm__ __volatile__(
		""  "subl %1,%0"
		:"=m" ((*(volatile struct { int a[100]; } *) v ) )
		:"ir" (i), "m" ((*(volatile struct { int a[100]; } *) v ) ));
}


union task_union {
	struct task_struct task;
	unsigned long stack[2048];
};

extern union task_union init_task_union;

extern struct   mm_struct init_mm;
extern struct task_struct *task[512 ];

extern struct task_struct **tarray_freelist;
extern spinlock_t taskslot_lock;

extern __inline__ void add_free_taskslot(struct task_struct **t)
{
	(void)( &taskslot_lock ) ;
	*t = (struct task_struct *) tarray_freelist;
	tarray_freelist = t;
	do { } while(0) ;
}

extern __inline__ struct task_struct **get_free_taskslot(void)
{
	struct task_struct **tslot;

	(void)( &taskslot_lock ) ;
	if((tslot = tarray_freelist) != ((void *)0) )
		tarray_freelist = (struct task_struct **) *tslot;
	do { } while(0) ;

	return tslot;
}




















# 1 "/home/scott/wrk/driver/linux/include/linux/module.h" 1









# 1 "/home/scott/wrk/driver/linux/include/linux/config.h" 1



# 1 "/home/scott/wrk/driver/linux/include/linux/autoconf.h" 1






















































































































































































































































































































































































































































































































































# 4 "/home/scott/wrk/driver/linux/include/linux/config.h" 2



# 10 "/home/scott/wrk/driver/linux/include/linux/module.h" 2













# 1 "/home/scott/wrk/driver/linux/include/asm/atomic.h" 1
























typedef struct { int counter; } atomic_t;







static __inline__ void atomic_add(int i, volatile atomic_t *v)
{
	__asm__ __volatile__(
		""  "addl %1,%0"
		:"=m" ((*(volatile struct { int a[100]; } *) v ) )
		:"ir" (i), "m" ((*(volatile struct { int a[100]; } *) v ) ));
}

static __inline__ void atomic_sub(int i, volatile atomic_t *v)
{
	__asm__ __volatile__(
		""  "subl %1,%0"
		:"=m" ((*(volatile struct { int a[100]; } *) v ) )
		:"ir" (i), "m" ((*(volatile struct { int a[100]; } *) v ) ));
}
