#include <stdio.h>
#include <stdlib.h>

int main(){
    char phone[11];
    int num;

    scanf("%s", &phone);

    while(scanf("%d", &num) != EOF){
        if (num == -1){
            for (int i = 0; i < 10; i++){
                printf("%c", phone[i]);
            }
            printf("\n");
        }
        else if (num >= 0 && num <=9){
            printf("%c\n", phone[num]);
        }
        else {
            printf("ERROR\n");
        }
    }
    return 0;
}