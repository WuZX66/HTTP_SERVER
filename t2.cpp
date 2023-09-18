#include <iostream>
#include <string.h>

void func(int ret)
{
    if (ret < 0)
    {
        fprintf(stderr, "dd");
        exit(-1);
    }
}
 
int main()
{
    int ret = -1;
    func(ret);
    printf("dsdsda");
    return 0;
}