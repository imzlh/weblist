#include<stdio.h>  

void main() {
    char str[20];

    printf("Enter string: ");
    gets(str);//reads string from console  
    printf("String is: %s", str);
    printf("
Lower String is: %s", strlwr(str));

}
//更多请阅读：https://www.yiibai.com/cprogramming/c-strlwr.html


