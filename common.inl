#define arr_len(arr)   (sizeof(arr) / sizeof(arr[0]))
#define max(x, y)      ((x) > (y) ? (x) : (y))
#define min(x, y)      ((x) < (y) ? (x) : (y))
#define clamp(a, x, y) (max(min((a), (y)), (x)))