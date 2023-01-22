#pragma once

#define PAREN ()

// recursively expand macros up to 324 times https://www.scs.stanford.edu/~dm/blog/va-opt.html
#define EXPAND(...) EXPAND1(EXPAND1(EXPAND1(EXPAND1(__VA_ARGS__))))
#define EXPAND1(...) EXPAND2(EXPAND2(EXPAND2(EXPAND2(__VA_ARGS__))))
#define EXPAND2(...) EXPAND3(EXPAND3(EXPAND3(EXPAND3(__VA_ARGS__))))
#define EXPAND3(...) EXPAND4(EXPAND4(EXPAND4(EXPAND4(__VA_ARGS__))))
#define EXPAND4(...) __VA_ARGS__

#ifdef __GNUC__
typedef __INT8_TYPE__ i8;
typedef __INT16_TYPE__ i16;
typedef __INT32_TYPE__ i32;
typedef __INT64_TYPE__ i64;

typedef __UINT8_TYPE__ u8;
typedef __UINT16_TYPE__ u16;
typedef __UINT32_TYPE__ u32;
typedef __UINT64_TYPE__ u64;

typedef unsigned int uint;
typedef const char* cstr;

typedef float f32;
typedef double f64;

#define I8_MAX __INT8_MAX__
#define I16_MAX __INT16_MAX__
#define I32_MAX __INT32_MAX__
#define I64_MAX __INT64_MAX__

#define U8_MAX __UINT8_MAX__
#define U16_MAX __UINT16_MAX__
#define U32_MAX __UINT32_MAX__
#define U64_MAX __UINT64_MAX__
#endif

#define true 1
#define false 0
#define STRING_LENGTH 64

#define vector(TYPE) vector_##TYPE
#define vector_init(TYPE) vector_init_##TYPE
#define vector_destroy(TYPE) vector_destroy_##TYPE
#define vector_at(TYPE) vector_at_##TYPE
#define vector_add(TYPE) vector_add_##TYPE
#define vector_rm(TYPE) vector_rm_##TYPE
#define vector_resize(TYPE) vector_resize_##TYPE

#define queue(TYPE) queue_##TYPE
#define queue_init(TYPE) queue_init_##TYPE
#define queue_destroy(TYPE) queue_destroy_##TYPE
#define queue_at(TYPE) queue_at_##TYPE
#define queue_push(TYPE) queue_push_##TYPE
#define queue_pop(TYPE) queue_pop_##TYPE

#define VECTOR_DECLARE(TYPE)\
typedef struct vector(TYPE) vector(TYPE)

#define VECTOR_DECLARE_INIT(TYPE) \
void vector_init(TYPE)(vector(TYPE)* v, u8* data, u32 size)

#define VECTOR_DECLARE_DESTROY(TYPE) \
void vector_destroy(TYPE)(vector(TYPE)* v)

#define VECTOR_DECLARE_AT(TYPE) \
TYPE* vector_at(TYPE)(vector(TYPE)* v, u32 index)

#define VECTOR_DECLARE_ADD(TYPE) \
void vector_add(TYPE)(vector(TYPE)* v, TYPE* data)

#define VECTOR_DECLARE_RM(TYPE) \
TYPE* vector_rm(TYPE)(vector(TYPE)* v)

#define VECTOR_DECLARE_RESIZE(TYPE) \
void vector_resize(TYPE)(vector(TYPE)* v, u32 size)

#define VECTOR_DECLARE_FN__() VECTOR_DECLARE_FN_

#define VECTOR_DECLARE_FN_(TYPE, arg, ...) \
VECTOR_DECLARE_##arg(TYPE); \
__VA_OPT__(VECTOR_DECLARE_FN__ PAREN (TYPE, __VA_ARGS__))

#define VECTOR_DECLARE_FN(TYPE, ...) \
__VA_OPT__(EXPAND(VECTOR_DECLARE_FN_(TYPE, __VA_ARGS__)))

#define QUEUE_DECLARE(TYPE) \
typedef struct queue(TYPE) queue(TYPE)

#define QUEUE_DECLARE_INIT(TYPE) \
void queue_init(TYPE)(queue(TYPE)* q, u8* data, u32 size)

#define QUEUE_DECLARE_DESTROY(TYPE) \
void queue_destroy(TYPE)(queue(TYPE)* q)

#define QUEUE_DECLARE_AT(TYPE) \
TYPE* queue_at(TYPE)(queue(TYPE)* q, TYPE* data)

#define QUEUE_DECLARE_PUSH(TYPE) \
void queue_push(TYPE)(queue(TYPE)* q, TYPE* data)

#define QUEUE_DECLARE_POP(TYPE) \
TYPE* queue_pop(TYPE)(queue(TYPE)* q, TYPE* data)

#define QUEUE_DECLARE_FN__() QUEUE_DECLARE_FN_

#define QUEUE_DECLARE_FN_(TYPE, arg, ...) \
QUEUE_DECLARE_##arg(TYPE); \
__VA_OPT__(QUEUE_DECLARE_FN__ PAREN (TYPE, __VA_ARGS__))

#define QUEUE_DECLARE_FN(TYPE, ...) \
__VA_OPT__(EXPAND(QUEUE_DECLARE_FN_(TYPE, __VA_ARGS__)))

#define VECTOR_DEFINE(TYPE) \
struct vector_##TYPE {    \
    TYPE* data;  \
    u32 size;    \
    u32 reserve; \
}

#define VECTOR_DEFINE_INIT(TYPE) \
void vector_init(TYPE)(vector(TYPE)* v, u8* data, u32 size) { \
	*v = {}; \
	v->data = (TYPE*)data; \
	v->reserve = size; \
}

#define VECTOR_DEFINE_DESTROY(TYPE) \
void vector_destroy(TYPE)(vector(TYPE)* v) { \
	free(v->data); \
	*v = {}; \
}

#define VECTOR_DEFINE_AT(TYPE) \
TYPE* vector_at(TYPE)(vector(TYPE)* v, u32 index) { \
	return v->data + index; \
}

// require definition of VEC_MEMCOPY, vector_resize
#define VECTOR_DEFINE_ADD(TYPE) \
void vector_add(TYPE)(vector(TYPE)* v, TYPE* data) {\
    if (v->reserve <= v->size) \
        vector_resize(TYPE)(v, v->size * 2); \
	VEC_MEMCPY((u8*)vector_at(TYPE)(v, v->size), (u8*)data, sizeof(TYPE)); \
	v->size++; \
}

#define VECTOR_DEFINE_RM(TYPE) \
TYPE* vector_rm(TYPE)(vector(TYPE)* v) { \
	v->size--; \
	return vec_at(TYPE)(v, v->size); \
}

// require definition of VEC_ALLOC and VEC_CPY
// always copies data
#define VECTOR_DEFINE_RESIZE(TYPE) \
void vector_resize(TYPE)(vector(TYPE)* v, u32 size) {\
	vector(u8) dst = { VEC_ALLOC(size * sizeof(TYPE)), sizeof(TYPE), size }; \
	vector(u8) src = { (u8*)v->data, sizeof(TYPE), v->size }; \
	VEC_CPY(&dst, &src); \
	free(v->data); \
	v->data = (TYPE*)dst.data; \
	v->reserve = size; \
}

#define VECTOR_DECLARE_FN__() VECTOR_DECLARE_FN_

#define VECTOR_DECLARE_FN_(TYPE, arg, ...) \
VECTOR_DECLARE_##arg(TYPE); \
__VA_OPT__(VECTOR_DECLARE_FN__ PAREN (TYPE, __VA_ARGS__))

#define VECTOR_DECLARE_FN(TYPE, arg, ...) \
__VA_OPT__(EXPAND(VECTOR_DECLARE_FN_(TYPE, __VA_ARGS__)))

#define QUEUE_DEFINE(TYPE) \
struct queue_##TYPE { \
	vector(TYPE) data; \
	u32 beg; \
	u32 end; \
}

#define QUEUE_DEFINE_INIT(TYPE) \
void queue_init(TYPE)(queue(TYPE)* q, u8* data, u32 size) { \
	*q = {}; \
	q->data.data = (TYPE*)data; \
	q->data.reserve = size; \
}

#define QUEUE_DEFINE_DESTROY(TYPE) \
void queue_destroy(TYPE)(queue(TYPE)* q) { \
	vector_destroy(TYPE)(&q->data); \
	*q = {}; \
}

#define QUEUE_DEFINE_AT(TYPE) \
TYPE* queue_at(TYPE)(queue(TYPE)* q, u32 index) { \
	return vector_at(TYPE)(&q->data, index); \
}

// see requirements for vector_add
#define QUEUE_DEFINE_PUSH(TYPE) \
void queue_push(TYPE)(queue(TYPE)* q, TYPE* data) { \
	if (q->data.reserve <= q->data.size) { \
		vector_resize(TYPE)(&q->data, q->data.size * 2); \
		q->end = q->data.size; \
	} \
	VEC_MEMCPY((u8*)queue_at(TYPE)(q, q->end), (u8*)data, sizeof(TYPE)); \
	q->end = (q->end + 1) % q->data.reserve; \
	q->data.size++; \
}

// see requirements for vector_rm
#define QUEUE_DEFINE_POP(TYPE) \
TYPE* queue_pop(TYPE)(queue(TYPE)* q) { \
	assert(q->begin <= q->end); \
	q->data.size--; \
	u32 tmp = q->begin++; \
	return queue_at(TYPE)(q, tmp); \
}

#define QUEUE_DEFINE_FN__() QUEUE_DEFINE_FN_

#define QUEUE_DEFINE_FN_(TYPE, arg, ...) \
QUEUE_DEFINE_##arg(TYPE); \
__VA_OPT__(QUEUE_DEFINE_FN__ PAREN (TYPE, __VA_ARGS__))

#define QUEUE_DEFINE_FN(TYPE, ...) \
__VA_OPT__(EXPAND(QUEUE_DEFINE_FN_(TYPE, __VA_ARGS__)))

// -1 if a < b
// +1 if a > b
// 0 if a == b
typedef int (*pfn_cmp)(u8* a, u8* b);
typedef void (*pfn_swap)(u8* a, u8* b);

typedef struct sort_params sort_params;

// data.size: size of each data block
// data.reserve: number of elements to sort
// actual reserved space should always be odd so heap operations do not access oob memory
// comparisons with null value should be valid
struct sort_params
{
	vector(u8) data;
	u8* null_value;
	pfn_cmp cmp;
	pfn_swap swap;
	u16 max_depth
	u16 insertion_thresh;
	u16 depth;
};

void vector_sort(sort_params* params);
void heap(sort_params* params);
void heap_add(sort_params* params); // assumes new data already appended to end of array
void heap_del(sort_params* params, u32 i);
void heap_rm(sort_params* params); // will set root to last element, remove with pop()

VECTOR_DEFINE(i8);
VECTOR_DECLARE_FN(i8, AT, ADD, RM, RESIZE);
VECTOR_DEFINE(i16);
VECTOR_DECLARE_FN(i16, AT, ADD, RM, RESIZE);
VECTOR_DEFINE(i32);
VECTOR_DECLARE_FN(i32, AT, ADD, RM, RESIZE);
VECTOR_DEFINE(i64);
VECTOR_DECLARE_FN(i64, AT, ADD, RM, RESIZE);

VECTOR_DEFINE(u8);
VECTOR_DECLARE_FN(u8, AT, ADD, RM, RESIZE);
VECTOR_DEFINE(u16);
VECTOR_DECLARE(u16, AT, ADD, RM, RESIZE);
VECTOR_DEFINE(u32);
VECTOR_DECLARE_FN(u32, AT, ADD, RM, RESIZE);
VECTOR_DEFINE(u64);
VECTOR_DECLARE_FN(u64, AT, ADD, RM, RESIZE);

VECTOR_DEFINE(f32);
VECTOR_DECLARE_FN(f32, AT, ADD, RM, RESIZE);
VECTOR_DEFINE(f64);
VECTOR_DECLARE_FN(f64, AT, ADD, RM, RESIZE);

enum
{
	ALLOC_NEW = 1 << 31,

	ATOMIC_MUTEX = 1 << 0,
	ATOMIC_TRANSFER = 1 << 1,
	ATOMIC_RESIZE = 1 <<2,
};

int spinlock_wait(_Atomic(u32)* lock, u32 target);
int spinlock_trywait(_Atomic(u32)* lock, u32 target);
int spinlock_nolock(_Atomic(u32)* lock, u32 target);
void spinlock_signal(_Atomic(u32)* lock, u32 target);

#define atomic(TYPE) atomic_##TYPE

#define atomic_vector(TYPE) atomic_vector_##TYPE
#define atomic_vector_init(TYPE) atomic_vector_init_##TYPE
#define atomic_vector_destroy(TYPE) atomic_vector_destroy_##TYPE
#define atomic_vector_load(TYPE) atomic_vector_load_##TYPE
#define atomic_vector_store(TYPE) atomic_vector_store_##TYPE
#define atomic_vector_add(TYPE) atomic_vector_add_##TYPE
#define atomic_vector_rm(TYPE) atomic_vector_rm_##TYPE
#define atomic_vector_resize(TYPE) atomic_vector_resize_##TYPE

#define atomic_queue(TYPE) atomic_queue_##TYPE
#define atomic_queue_init(TYPE) atomic_queue_init_##TYPE
#define atomic_queue_destroy(TYPE) atomic_queue_destroy_##TYPE
#define atomic_queue_load(TYPE) atomic_queue_load_##TYPE
#define atomic_queue_store(TYPE) atomic_queue_store_##TYPE
#define atomic_queue_push(TYPE) atomic_queue_push_##TYPE
#define atomic_queue_pop(TYPE) atomic_queue_pop_##TYPE
#define atomic_queue_trypop(TYPE) atomic_queue_pop_##TYPE

#define ATOMIC_VECTOR_DECLARE(TYPE)\
typedef struct atomic_vector(TYPE) atomic_vector(TYPE)

#define ATOMIC_VECTOR_DECLARE_INIT(TYPE) \
void atomic_vector_init(TYPE)(atomic_vector(TYPE)* v, u8* data, u32 size)

#define ATOMIC_VECTOR_DECLARE_DESTROY(TYPE) \
void atomic_vector_destroy(TYPE)(atomic_vector(TYPE)* v)

#define ATOMIC_VECTOR_DECLARE_LOAD(TYPE) \
void atomic_vector_load(TYPE)(atomic_vector(TYPE)* v, u32 index, TYPE* out)

#define ATOMIC_VECTOR_DECLARE_STORE(TYPE) \
void atomic_vector_store(TYPE)(atomic_vector(TYPE)* v, u32 index, TYPE* in)

#define ATOMIC_VECTOR_DECLARE_ADD(TYPE) \
void atomic_vector_add(TYPE)(atomic_vector(TYPE)* v, TYPE* data)

#define ATOMIC_VECTOR_DECLARE_RM(TYPE) \
u32 atomic_vector_rm(TYPE)(atomic_vector(TYPE)* v)

#define ATOMIC_VECTOR_DECLARE_RESIZE(TYPE) \
void atomic_vector_resize(TYPE)(atomic_vector(TYPE)* v, u32 size)

#define ATOMIC_VECTOR_DECLARE_FN__() ATOMIC_VECTOR_DECLARE_FN_

#define ATOMIC_VECTOR_DECLARE_FN_(TYPE, arg, ...) \
ATOMIC_VECTOR_DECLARE_##arg(TYPE); \
__VA_OPT__(ATOMIC_VECTOR_DECLARE_FN__ PAREN (TYPE, __VA_ARGS__))

#define ATOMIC_VECTOR_DECLARE_FN(TYPE, ...) \
__VA_OPT__(EXPAND(ATOMIC_VECTOR_DECLARE_FN_(TYPE, __VA_ARGS__)))

#define ATOMIC_QUEUE_DECLARE(TYPE) \
typedef struct atomic_queue(TYPE) atomic_queue(TYPE)

#define ATOMIC_QUEUE_DECLARE_INIT(TYPE) \
void atomic_queue_init(TYPE)(atomic_queue(TYPE)* q, u8* data, u32 size)

#define ATOMIC_QUEUE_DECLARE_DESTROY(TYPE) \
void atomic_queue_destroy(TYPE)(atomic_queue(TYPE)* q)

#define ATOMIC_QUEUE_DECLARE_LOAD(TYPE) \
void atomic_queue_load(TYPE)(atomic_queue(TYPE)* q, u32 index, TYPE* out)

#define ATOMIC_QUEUE_DECLARE_STORE(TYPE) \
void atomic_queue_store(TYPE)(atomic_queue(TYPE)* q, u32 index, TYPE* in)

#define ATOMIC_QUEUE_DECLARE_PUSH(TYPE) \
void atomic_queue_push(TYPE)(atomic_queue(TYPE)* q, TYPE* data)

#define ATOMIC_QUEUE_DECLARE_POP(TYPE) \
int atomic_queue_pop(TYPE)(atomic_queue(TYPE)* q, TYPE* out)

#define ATOMIC_QUEUE_DECLARE_TRYPOP(TYPE) \
void atomic_queue_trypop(TYPE)(atomic_queue(TYPE)* q, TYPE* out)

#define ATOMIC_QUEUE_DECLARE_FN__() ATOMIC_QUEUE_DECLARE_FN_

#define ATOMIC_QUEUE_DECLARE_FN_(TYPE, arg, ...) \
ATOMIC_QUEUE_DECLARE_##arg(TYPE); \
__VA_OPT__(ATOMIC_QUEUE_DECLARE_FN__ PAREN (TYPE, __VA_ARGS__))

#define ATOMIC_QUEUE_DECLARE_FN(TYPE, ...) \
__VA_OPT__(EXPAND(ATOMIC_QUEUE_DECLARE_FN_(TYPE, __VA_ARGS__)))

#define ATOMIC_VECTOR_DEFINE(TYPE) \
struct atomic_vector_##TYPE {    \
    _Atomic(TYPE*) data;  \
    _Atomic(u32) size;    \
    _Atomic(u32) reserve; \
	_Atomic(u32) flags;
}

#define VECTOR_DEFINE_INIT(TYPE) \
void atomic_vector_init(TYPE)(atomic_vector(TYPE)* v, u8* data, u32 size) { \
	atomic_init(&v->data, data); \
	atomic_init(&v->size, 0); \
	atomic_init(&v->reserve, size); \
	atomic_init(&v->flags, 0);\
}

// assumes that only the calling thread has access to the vector, and all other operations are completed
#define ATOMIC_VECTOR_DEFINE_DESTROY(TYPE) \
void atomic_vector_destroy(TYPE)(atomic_vector(TYPE)* v) { \
	atomic_store_explicit(&v->size, 0, memory_order_relaxed); \
	atomic_store_explicit(&v->reserve, 0, memory_order_relaxed); \
	atomic_store_explicit(&v->flags, 0, memory_order_relaxed); \
	TYPE* buf = atomic_exchange_explicit(&v->data, NULL, memory_order_relaxed); \
	free(buf); \
}

// need VEC_MEMCOPY
#define ATOMIC_VECTOR_DEFINE_LOAD(TYPE) \
void atomic_vector_load(TYPE)(atomic_vector(TYPE)* v, u32 index, TYPE* out) { \
	TYPE* buf = atomic_load_explicit(&v->data, memory_order_acquire); \
	VEC_MEMCPY((u8*)out, (u8*)(buf + index), sizeof(TYPE)); \
}

#define ATOMIC_VECTOR_DEFINE_STORE(TYPE) \
void atomic_vector_store(TYPE)(atomic_vector(TYPE)* v, u32 index, TYPE* in) { \
	spinlock_nolock(&v->flags, ATOMIC_RESIZE); \
	TYPE* buf = atomic_load_explicit(&v->data, memory_order_acquire); \
	VEC_MEMCPY((u8*)(buf + index), (u8*)in, sizeof(TYPE)); \
}

// require definition of VEC_MEMCPY, vector_resiz
#define ATOMIC_VECTOR_DEFINE_ADD(TYPE) \
void atomic_vector_add(TYPE)(vector(TYPE)* v, TYPE* data) {\
	u32 size = atomic_load_explicit(&v->size, memory_order_relaxed); \
	while (!atomic_compare_exchange_weak_explicit(&v->size, &size, size + 1, memory_order_release, memory_order_relaxed)); \
	u32 reserve = atomic_load_explicit(&v->reserve, memory_order_acquire); \
    if (reserve < size) \
        atomic_vector_resize(TYPE)(v, v->size * 2); \
	atomic_vector_store(TYPE)(v, size, data); \
}

#define ATOMIC_VECTOR_DEFINE_RM(TYPE) \
u32 atomic_vector_rm(TYPE)(atomic_vector(TYPE)* v) { \
	u32 size = atomic_load_explicit(&v->size, memory_order_relaxed); \
	while (!atomic_compare_exchange_weak_explicit(&v->size, &size, size - 1, memory_order_release, memory_order_relaxed)); \
	return size - 1; \
}

// require definition of VEC_ALLOC and VEC_CPY
// always copies data
#define ATOMIC_VECTOR_DEFINE_RESIZE(TYPE) \
void atomic_vector_resize(TYPE)(atomic_vector(TYPE)* v, u32 size) {\
	if (!spinlock_wait(&v->flags, ATOMIC_RESIZE)) {\
		spinlock_signal(&v->flags, ATOMIC_RESIZE); \
		return; \
	} \
	u32 vsize = atomic_load_explicit(&v->size, memory_order_acquire); \
	TYPE* old = atomic_load_explicit(&v->data, memory_order_relax); \
	TYPE* buf = (TYPE*)VEC_ALLOC(size * sizeof(TYPE)); \
	vector(u8) dst = { buf, sizeof(TYPE), size }; \
	vector(u8) src = { old, sizeof(TYPE), vsize }; \
	VEC_CPY(&dst, &src); \
	atomic_store_explicit(&v->data, buf, memory_order_release); \
	atomic_store_explicit(&v->reserve, size, memory_order_release); \
	spinlock_signal(&v->flags, ATOMIC_RESIZE); \
	free(v->data); \
}

#define VECTOR_DECLARE_FN__() VECTOR_DECLARE_FN_

#define VECTOR_DECLARE_FN_(TYPE, arg, ...) \
VECTOR_DECLARE_##arg(TYPE); \
__VA_OPT__(VECTOR_DECLARE_FN__ PAREN (TYPE, __VA_ARGS__))

#define VECTOR_DECLARE_FN(TYPE, arg, ...) \
__VA_OPT__(EXPAND(VECTOR_DECLARE_FN_(TYPE, __VA_ARGS__)))

#define ATOMIC_QUEUE_DEFINE(TYPE) \
struct atomic_queue_##TYPE { \
	atomic_vector(TYPE) data; \
	_Atomic(u32) beg; \
	_Atomic(u32) end; \
}

#define ATOMIC_QUEUE_DEFINE_INIT(TYPE) \
void atomic_queue_init(TYPE)(atomic_queue(TYPE)* q, u8* data, u32 size) { \
	atomic_vector_init(TYPE)(&q->data, data, size); \
	atomic_init(&q->beg, 0); \
	atomic_init(&q->end, 0); \
}

#define ATOMIC_QUEUE_DEFINE_DESTROY(TYPE) \
void atomic_queue_destroy(TYPE)(atomic_queue(TYPE)* q) { \
	atomic_vector_destroy(TYPE)(&q->data); \
	atomic_store_explicit(&q->beg, 0, memory_order_relaxed); \
	atomic_store_explicit(&q->end, 0, memory_order_relaxed); \
}

#define ATOMIC_QUEUE_DEFINE_LOAD(TYPE) \
void atomic_queue_load(TYPE)(atomic_queue(TYPE)* q, u32 index, TYPE* out) { \
	atomic_vector_load(TYPE)(&q->data, index, out); \
}

#define ATOMIC_QUEUE_DEFINE_STORE(TYPE) \
void atomic_queue_store(TYPE)(atomic_queue(TYPE)* q, u32 index, TYPE* in) { \
	atomic_vector_store(TYPE)(&q->data, index, in); \
}

// see requirements for vector_add
#define ATOMIC_QUEUE_DEFINE_PUSH(TYPE) \
void atomic_queue_push(TYPE)(atomic_queue(TYPE)* q, TYPE* data) { \
	u32 size = atomic_load_explicit(&q->data.size, memory_order_relaxed); \
	u32 reserve = atomic_load_explicit(&q->data.reserve, memory_order_relaxed); \
	if (reserve <= size) \
		atomic_vector_resize(TYPE)(&q->data, size * 2); \
	while (!atomic_compare_exchange_weak_explicit(&q->data.size, &size, size + 1, memory_order_release, memory_order_relaxed)); \
	u32 end = atomic_load_explicit(&q->end, memory_order_relaxed); \
	while (!atomic_compare_exchange_weak_explicit(&q->end, &end, end + 1, memory_order_release, memory_order_relaxed)); \
	atomic_queue_store(q, end, data); \
}

// see requirements for vector_rm
#define ATOMIC_QUEUE_DEFINE_POP(TYPE) \
void atomic_queue_pop(TYPE)(atomic_queue(TYPE)* q, TYPE* out) { \
	u32 beg = atomic_load_explicit(&q->beg, memory_order_relaxed); \
	while (!atomic_compare_exchange_weak_explicit(&q->beg, &beg, beg + 1, memory_order_release, memory_order_relaxed)); \
	atomic_queue_load(TYPE)(q, beg, out); \
	atomic_fetch_sub_explicit(&q->data.size, 1, memory_order_relaxed); \
}

// see requirements for vector_rm
#define ATOMIC_QUEUE_DEFINE_TRYPOP(TYPE) \
int atomic_queue_trypop(TYPE)(atomic_queue(TYPE)* q, TYPE* out) { \
	u32 beg = atomic_load_explicit(&q->beg, memory_order_acquire); \
	u32 end = atomic_load_explicit(&q->end, memory_order_acquire); \
	if (end <= beg) return 0; \
	while (!atomic_compare_exchange_weak_explicit(&q->beg, &beg, beg + 1, memory_order_release, memory_order_relaxed)); \
	atomic_queue_load(TYPE)(q, beg, out); \
	atomic_fetch_sub_explicit(&q->data.size, 1, memory_order_relaxed); \
	return 1; \
}

#define ATOMIC_QUEUE_DEFINE_FN__() ATOMIC_QUEUE_DEFINE_FN_

#define ATOMIC_QUEUE_DEFINE_FN_(TYPE, arg, ...) \
ATOMIC_QUEUE_DEFINE_##arg(TYPE); \
__VA_OPT__(ATOMIC_QUEUE_DEFINE_FN__ PAREN (TYPE, __VA_ARGS__))

#define ATOMIC_QUEUE_DEFINE_FN(TYPE, ...) \
__VA_OPT__(EXPAND(ATOMIC_QUEUE_DEFINE_FN_(TYPE, __VA_ARGS__)))

typedef atomic_vector(u8) atomic_baseptr;
typedef vector(u8) baseptr;
typedef struct mem_block mem_block;
typedef struct atomic_mem_pool atomic_mem_pool;
typedef struct atomic_mem_list atomic_mem_list;

struct mem_block
{
	baseptr* base;
	u32 handle;
	u32 size;
};

struct atomic_mem_pool
{
	atomic_baseptr base;
	_Atomic(u32) free;
};

ATOMIC_VECTOR_DECLARE(atomic_baseptr);
ATOMIC_VECTOR_DEFINE(atomic_baseptr);

struct atomic_mem_list
{
	atomic_vector(baseptr) blocks;
	_Atomic(u64) free;
};

#define atomic_mem_load(TYPE) atomic_mem_load_##TYPE
#define atomic_mem_store(TYPE) atomic_mem_store##TYPE

// require VEC_MEMCPY
#define ATOMIC_MEM_LOAD_DEFINE(TYPE) \
void atomic_mem_load(TYPE)(mem_block* block, TYPE* out) { \
	atomic_baseptr* base = (atomic_baseptr*)block->base; \
	TYPE* buf = (TYPE*)atomic_load_explicit(&base->data, memory_order_relaxed); \
	VEC_MEMCPY((u8*)out, (u8*)(buf + block->handle), sizeof(TYPE)); \
}

#define ATOMIC_MEM_STORE_DEFINE(TYPE) \
void atomic_mem_store(TYPE)(mem_block* block, TYPE* in) { \
	atomic_baseptr* base = (atomic_baseptr*)block->base; \
	spinlock_nolock(&base->flags, ATOMIC_RESIZE); \
	TYPE* buf = (TYPE*)atomic_load_explicit(&base->data, memory_order_acquire); \
	VEC_MEMCPY((u8*)(buf + block->handle), (u8*)in, sizeof(TYPE)); \
}

void atomic_pool_init(atomic_mem_pool* pool, u32 size, u32 blocksz);
void atomic_pool_destroy(atomic_mem_pool* pool);
void atomic_pool_resize(atomic_mem_pool* pool, u32 size);

mem_block atomic_pool_at(atomic_mem_pool* pool, u32 handle);
mem_block atomic_pool_alloc(atomic_mem_pool* pool);
void atomic_pool_free(mem_block* block);

void atomic_list_init(atomic_mem_list* list, u32 size, u32 blocksz);
void atomic_list_destroy(atomic_mem_list* list);
void atomic_list_resize(atomic_mem_list* list, u32 size);

u8* atomic_list_at(atomic_mem_list* list, u64 handle);
u64 atomic_list_alloc(atomic_mem_list* list);
void atomic_list_free(atomic_mem_list* list, u64 handle);

const u32 PRIME_ARRAY[] = {
	187091u,     1289u,       28802401u,   149u,        15173u,      2320627u,    357502601u,  53u,
	409u,        4349u,       53201u,      658753u,     8175383u,    101473717u,  1259520799u, 19u,
	89u,         241u,        709u,        2357u,       8123u,       28411u,      99733u,      351061u,
	1236397u,    4355707u,    15345007u,   54061849u,   190465427u,  671030513u,  2364114217u, 7u,
	37u,         71u,         113u,        193u,        313u,        541u,        953u,        1741u,
	3209u,       5953u,       11113u,      20753u,      38873u,      72817u,      136607u,     256279u,
	480881u,     902483u,     1693859u,    3179303u,    5967347u,    11200489u,   21023161u,   39460231u,
	74066549u,   139022417u,  260944219u,  489790921u,  919334987u,  1725587117u, 3238918481u, 3u,
	13u,         29u,         43u,         61u,         79u,         103u,        137u,        167u,
	211u,        277u,        359u,        467u,        619u,        823u,        1109u,       1493u,
	2029u,       2753u,       3739u,       5087u,       6949u,       9497u,       12983u,      17749u,
	24281u,      33223u,      45481u,      62233u,      85229u,      116731u,     159871u,     218971u,
	299951u,     410857u,     562841u,     771049u,     1056323u,    1447153u,    1982627u,    2716249u,
	3721303u,    5098259u,    6984629u,    9569143u,    13109983u,   17961079u,   24607243u,   33712729u,
	46187573u,   63278561u,   86693767u,   118773397u,  162723577u,  222936881u,  305431229u,  418451333u,
	573292817u,  785430967u,  1076067617u, 1474249943u, 2019773507u, 2767159799u, 3791104843u, 2u,
	5u,          11u,         17u,         23u,         31u,         41u,         47u,         59u,
	67u,         73u,         83u,         97u,         109u,        127u,        139u,        157u,
	179u,        199u,        227u,        257u,        293u,        337u,        383u,        439u,
	503u,        577u,        661u,        761u,        887u,        1031u,       1193u,       1381u,
	1613u,       1879u,       2179u,       2549u,       2971u,       3469u,       4027u,       4703u,
	5503u,       6427u,       7517u,       8783u,       10273u,      12011u,      14033u,      16411u,
	19183u,      22447u,      26267u,      30727u,      35933u,      42043u,      49201u,      57557u,
	67307u,      78779u,      92203u,      107897u,     126271u,     147793u,     172933u,     202409u,
	236897u,     277261u,     324503u,     379787u,     444487u,     520241u,     608903u,     712697u,
	834181u,     976369u,     1142821u,    1337629u,    1565659u,    1832561u,    2144977u,    2510653u,
	2938679u,    3439651u,    4026031u,    4712381u,    5515729u,    6456007u,    7556579u,    8844859u,
	10352717u,   12117689u,   14183539u,   16601593u,   19431899u,   22744717u,   26622317u,   31160981u,
	36473443u,   42691603u,   49969847u,   58488943u,   68460391u,   80131819u,   93793069u,   109783337u,
	128499677u,  150406843u,  176048909u,  206062531u,  241193053u,  282312799u,  330442829u,  386778277u,
	452718089u,  529899637u,  620239453u,  725980837u,  849749479u,  994618837u,  1164186217u, 1362662261u,
	1594975441u, 1866894511u, 2185171673u, 2557710269u, 2993761039u, 3504151727u, 4101556399u
};

u32 hash_value(u8* key, int len);
u32 hash_size(u32 size);

#define compare(TYPE) compare_##TYPE
#define COMPARE_DECLARE(TYPE) \
int compare(TYPE)(u8* a, u8* b)

#define hash_table(K, V) hash_table_##K_##V
#define key_item(K) key_item_##K
#define table_init(K, V) table_init_##K_##V
#define table_destroy(K, V) table_destroy_##K_##V
#define table_add(K, V) table_add_##K_##V
#define table_find(K, V) table_find_##K_##V
#define table_resize(K, V) table_resize_##K_##V

#define HASH_TABLE_DECLARE(K, V) \
typedef struct hash_table(K, V) hash_table(K, V); \
typedef struct key_item(K) key_item(K)

#define HASH_TABLE_DECLARE_INIT(K, V) \
void table_init(K, V)(hash_table(K, V)* table, u32 size)

#define HASH_TABLE_DECLARE_DESTROY(K, V) \
void table_destroy(K, V)(hash_table(K, V)* table)

#define HASH_TABLE_DECLARE_ADD(K, V) \
void table_add(K, V)(hash_table(K, V)* table, K* key, V* value)

#define HASH_TABLE_DECLARE_FIND(K, V) \
V* table_find(K, V)(hash_table(K, V)* table, K* key)

#define HASH_TABLE_DECLARE_RESIZE(K, V) \
void table_resize(K, V)(hash_table(K, V)* table, u32 size)

#define HASH_TABLE_DECLARE_FN__() HASH_TABLE_DECLARE_FN_

#define HASH_TABLE_DECLARE_FN_(K, V, arg, ...) \
HASH_TABLE_DECLARE_##arg(K, V); \
__VA_OPT__(HASH_TABLE_DECLARE_FN__ PAREN (TYPE, __VA_ARGS__))

#define HASH_TABLE_DECLARE_FN(K, V, arg, ...) \
__VA_OPT__(EXPAND(HASH_TABLE_DECLARE_FN_(K, V, __VA_ARGS__)))

#define HASH_TABLE_DEFINE(K, V) \
struct key_item(K) {\
	u32 hash; \
	K key; \
}; \
struct hash_table(K, V) { \
	vector(key_item(K)) keys; \
	vector(V) items; \
	u32 max_probe; \
	u32 probe; \
}

#define HASH_TABLE_DEFINE_INIT(K, V) \
void table_init(K, V)(hash_table(K, V)* table, u32 size) { \
	size = hash_size(size); \
	key_item(K)* key = (key_item(K)*)jolly_alloc(size * sizeof(key_item(K))); \
	V* value = (V*)jolly_alloc(size * sizeof(V)); \
	vector_init(key_item(K))(&table->keys, key, size); \
	vector_init(V)(&table->items, value, size); \
	table->max_probe = 24; \
	table->probe = 0; \
}

#define HASH_TABLE_DEFINE_ADD(K, V) \
void table_add(K, V)(hash_table(K, V)* table, K* key, V* value) { \
	if (table->max_probe < table->probe || table->keys.reserve <= table->keys.size) \
		table_resize(K, V)(table, table->keys.size * 2); \
	u32 reserve = table->keys.reserve; \
	u32 hash = hash_value((u8*)key, sizeof(K)); \
	u32 index = hash % reserve; u32 probe = 0; \
	key_item(K)* item = vector_at(key_item(K))(&table->keys, index); \
	key_item(K) tmpitem; V tmpval; \
	while (true) { \
		if (!item->hash) { \
			item->hash = hash; \
			item->key = *key; \
			vector_store(V)(&table->items, index, value); \
			table->keys.size++; \
			table->items.size++; \
			break; \
		} \
		u32 dist = ABS(item->hash % reserve - index); \
		if (dist < probe) { \
			tmpitem = *item; \
			item->hash = hash; \
			item->key = *key; \
			hash = tmpitem.hash; \
			key = &tmpitem.key; \
			vector_load(V)(&table->items, index, &tmpval); \
			vector_store(V)(&table->items, index, value); \
			value = &tmpval; \
			probe = dist; \
		} \
		probe++; \
		index = (index + 1) < reserve ? index + 1 : 0; \
		item = vector_at(key_item(K))(&table->keys, index); \
	} \
	table->probe = table->probe < probe ? probe : table->probe; \
}

#define HASH_TABLE_DEFINE_FIND(K, V) \
V* table_find(K, V)(hash_table(K, V)* table, K* key) { \
	u32 hash = hash_value((u8*)key, sizeof(K)); \
	u32 reserve = table->keys.reserve; \
	u32 index = hash % eserve; \
	key_item(K)* item = vector_at(key_item(K))(&table->keys, index); \
	for (u32 i = 0; i <= table->probe; i++) { \
		if (hash == item->hash && compare((u8*)key, (u8*)&item->key)) \
			return vector_at(V)(&table->items, index); \
		index = (index + 1) < reserve ? index + 1 : 0; \
	} \
	return NULL; \
}

#define HASH_TABLE_DEFINE_RESIZE(K, V) \
void table_resize(K, V)(hash_table(K, V)* table, u32 size) { \
	size = hash_size(size); \
	vector(key_item(K)) key = { jolly_alloc(size * sizeof(key_item(K))), 0, size }; \
	vector(V) val = { jolly_alloc(size * sizeof(V)), 0, size }; \
	table->probe = 0; \
	for (u32 i = 0; i < table->keys.reserve; i++) { \
		key_item(K)* item = vector_at(key_item(K))(&table->keys, i); \
		V* value = vector_at(V)(&table->items, i); \
		u32 index = item->hash % size; u32 probe = 0; \
		key_item(K)* nitem = vector_at(key_item(K))(&key, index); \
		key_item(K) tmpitem; V tmpval; \
		while (true) { \
			if (!nitem->hash) { \
				nitem->hash = item->hash; \
				nitem->key = item->key; \
				vector_store(V)(&val, index, value); \
				break; \
			} \
			u32 dist = ABS(nitem->hash % size - index); \
			if (dist < probe) { \
				tmpitem = *nitem; \
				nitem->hash = item->hash; \
				nitem->key = item->key; \
				*item = tmpitem; \
				vector_load(V)(&table->items, index, &tmpval); \
				vector_store(V)(&table->items, index, value); \
				value = &tmpval; \
				probe = dist; \
			} \
			probe++; \
			index = (index + 1) < size ? index + 1 : 0; \
			nitem = vector_at(key_item(K))(&table->keys, index); \
		} \
		table->probe = table->probe < probe ? probe : table->probe; \
	} \
	free(table->keys.data); \
	free(table->items.data); \
	table->keys.data = key.data; \
	table->items.data = val.data; \
}

enum
{
	BLOCK_16 = 16,
	BLOCK_32 = 32,
	BLOCK_64 = 64,
	BLOCK_128 = 128,
	BLOCK_256 = 256,
	BLOCK_512 = 512,
	BLOCK_1024 = 1024,
	BLOCK_2048 = 2048,
	BLOCK_4096 = 4096,
	BLOCK_8192 = 8192,
};

typedef vector(u8) baseptr;
typedef struct mem_block mem_block;
typedef struct mem_arena mem_arena;
typedef struct mem_pool mem_pool;
typedef struct mem_list mem_list;

struct mem_block
{
	baseptr* base;
	u32 handle;
	u32 size;
};

VECTOR_DECLARE(mem_block);
VECTOR_DEFINE(mem_block);

// supports dynamically sized allocations
// continuous buffer of memory
struct mem_arena
{
	baseptr base;
	vector(mem_block) heapgc;
	mem_block free;
};

// fixed size allocations
// continuous buffer of memory
struct mem_pool
{
	baseptr base;
	u32 free;
};

VECTOR_DECLARE(baseptr);
VECTOR_DEFINE(baseptr);

// fixed sized allocations, eight byte minimum allocations
// memory not continuous
struct mem_list
{
	vector(baseptr) blocks;
	u64 free;
};

inline u8* MEM_DATA(mem_block* block)
{
	baseptr* base = block->base;
	return vector_at(u8)(base, block->handle * base->size)
}

u8* jolly_alloc(size);

void arena_init(mem_arena* arena, u32 size, u32 blocksz);
void arena_destroy(mem_arena* arena);
void arena_resize(mem_arena* arena, u32 size);
void arena_gc(mem_arena* arena);

mem_block arena_alloc(mem_arena* arena, u32 count);
void arena_realloc(mem_block* block, u32 count);
void arena_free(mem_block* block);

void pool_init(mem_pool* pool, u32 size, u32 blocksz);
void pool_destroy(mem_pool* pool);
void pool_resize(mem_pool* pool, u32 size);

mem_block pool_alloc(mem_pool* pool);
void pool_free(mem_block* block);

void list_init(mem_list* list, u32 size, u32 blocksz);
void list_destroy(mem_list* list);
void list_resize(mem_list* list, u32 size);

mem_block list_alloc(mem_list* list);
void list_free(mem_list* list, mem_block* block);

void vector_cpy16(vector(u8)* dst, vector(u8)* src);
void vector_cpy32(vector(u8)* dst, vector(u8)* src);

void vector_set16(vector(u8)* dst, u32 val);
void vector_set32(vector(u8)* dst, u32 val);

int vector_cmp16(vector(u8)* dst, vector(u8)* src);
int vector_cmp32(vector(u8)* dst, vector(u8)* src);

// unaligned versions
void vector_cpy16u(vector(u8)* dst, vector(u8)* src);
void vector_cpy32u(vector(u8)* dst, vector(u8)* src);

int vector_cmp16u(vector(u8)* dst, vector(u8)* src);
int vector_cmp32u(vector(u8)* dst, vector(u8)* src);
