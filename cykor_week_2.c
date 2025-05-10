#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#define MAX 300
int pwd(char* token[], int num);
int cd(char* token[], int num);
void prindir(void);
void pipeline(char* cmds[][MAX],int cmd);
void goback(char* token[]) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    }
    if (pid == 0) {
        // 자식 프로세스: 백그라운드로 멀티/파이프라인 처리
        mulprom(token);
        exit(0);
    }
}
int dividepipe(char* token[]) //파이프라인 나누기
{
int a=0; // 파이프 개수
for(int i=0;token[i]!=NULL;i++)
{
	if(strcmp(token[i],"|")==0)   //문자열을 비교하기 위해 큰따옴표로 해야 한다고 함.
	{
		a++;
	}
}
int b = a+1; // b는 인자 개수
if(b>20)
{
	fprintf(stderr,"에러:파이프라인은은 최대 20개입니다.");
	return 0;
}
char* cmds[20][MAX]; // 각각의 인자들을 담는 배열
int cmd=0,cmdline=0; // cmd는 명령어 단위, cmdline 은 명령어 인자 
for (int i = 0; token[i] != NULL; i++) {
	if (strcmp(token[i], "|") == 0) {
		cmds[cmd][cmdline] = NULL;  // 현재 명령어 끝 표시
		cmd++;               
		cmdline = 0;            
	} else {
		cmds[cmd][cmdline++] = token[i];
	}
}
cmds[cmd][cmdline] = NULL; 
pipeline(cmds,b);
return 1;
// 마지막 명령어 끝 표시 << 이거 안하면 에러 남

// printf("파이프라인 개수: %d\n", a);
// for (int i = 0; i < b; i++) {
// 	printf("명령[%d]:", i);
// 	for (int j = 0; cmds[i][j] != NULL; j++) {
// 		printf(" %s", cmds[i][j]);
// 	}
// 	printf("\n");
// }


}
void pipeline(char* cmds[][MAX], int cmd) {
    int pipes = cmd - 1;
    int descrip[pipes * 2];

    for (int i = 0; i < pipes; i++) {
        if (pipe(descrip + 2*i) < 0) {
            perror("pipe");
			return;
        }
    }

    for (int c = 0; c < cmd; c++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
			return;
        }
        if (pid == 0) {
            if (c > 0)
                dup2(descrip[2*(c-1)], STDIN_FILENO);
            if (c < pipes)
                dup2(descrip[2*c + 1], STDOUT_FILENO);
            for (int j = 0; j < 2*pipes; j++)
                close(descrip[j]);
            execvp(cmds[c][0], cmds[c]);
            perror("execvp");
            exit(1);
        }
    }
	for (int j = 0; j < 2*pipes; j++)
	        close(descrip[j]);
	for (int c = 0; c < cmd; c++)
	        wait(NULL);
	 }




void addspace(char* line) //다중 명령어랑 파이프라인 양옆으로 공백 추가
{
	char buf[MAX * 2] = { 0 };
	int j = 0;
	for (int i = 0; line[i] != NULL; i++) {
		// && 또는 || 처리
		if ((line[i] == '&' && line[i + 1] == '&') ||
			(line[i] == '|' && line[i + 1] == '|')) 
		{
			buf[j++] = ' ';
			buf[j++] = line[i++];
			buf[j++] = line[i];
			buf[j++] = ' ';
		}
		// 단일 |, ;, , &처리
		else if (line[i] == '|' || line[i] == ';' || line[i] == ','||line[i] == '&')
		{
			buf[j++] = ' ';
			buf[j++] = line[i];
			buf[j++] = ' ';
		}
		else 
		{
			buf[j++] = line[i];
		}
	}
	buf[j] = NULL;
	strcpy(line, buf);
}
int yesnopipe(char* token[])
{
	int check1 = 0;
	int check2 = 0;
	for (int q = 0; token[q] != NULL; q++)
	{
		check1++;
	}
	for (int p = 0; token[p] != NULL; p++)
	{
		if (strcmp(token[p], "|") == 0)   // | 있으면 파이프로 ㄱㄱ
		{
			return  dividepipe(token);
			break;
		}
		check2++;
	}
	if (check1 == check2)   //  check1==check2 즉 파이프 없으면 그냥 없는걸로 ㄱㄱ
	{
		return prompt(token);
	}
}
int prompt(char* token[])
{

	int num = 0; //token 인자 몇개인지
	while (token[num] != NULL) //token 인자 개수 세기
	{
		num++;
	}
	for (int a = 0; token[a]!=NULL; a++)
	{
		if (num > 0 && strcmp(token[a], "cd") == 0)
		{
			return cd(token, num);
		}
		else if (num > 0 && strcmp(token[a], "pwd") == 0)
		{
			return pwd(token, num);
		}
		else { //외부 명령어 처리 없으면 오류
			pid_t pid = fork();
			if (pid < 0) {
				perror("fork");
				return 0;
			}
			if (pid == 0) {
				
				execvp(token[0], token);
				perror("execvp");  
				exit(1);
			}
			wait(NULL);
			return 1;
		}
	}
}
void mulprom(char* token[])    //다중 명령어 처리 = 다중 명령어 기준으로 왼쪽과 오른쪽으로 나눔
{
	int nomulti = 0;
	char* sorttok[MAX] = { 0 };
	char* sorttok2[MAX] = { 0 };
	for (int i = 0; token[i]!=NULL; i++)
	{
		int endcheck = 0;
		if (strcmp(token[i], "&&") == 0)
		{
			++nomulti;
			for (int a = 0; a < i; a++)
			{
				sorttok[a] = token[a];
			}

			for (int a = 0; token[a+i+1]!=  NULL; a++)
			{
				sorttok2[a] = token[a + i + 1];
				++endcheck;
			}
			sorttok[i] = NULL;   // 끝 체크
			sorttok2[endcheck] = NULL;
			int check;
			check = yesnopipe(sorttok);
			if (check == 1)
			{
				mulprom(sorttok2);
				break;
			}
			else break;

		}
		else if (strcmp(token[i], "||") == 0)
		{
			++nomulti;
			for (int a = 0; a < i; a++)
			{
				sorttok[a] = token[a];
			}

			for (int a = 0; token[a + i + 1] != NULL; a++)
			{
				sorttok2[a] = token[a + i + 1];
				++endcheck;
			}
			sorttok[i] = NULL;   // 끝 체크
			sorttok2[endcheck] = NULL;
			int check;
			check = yesnopipe(sorttok);
			if (check == 0)
			{
				mulprom(sorttok2);
				break;
			}
			else break;
		}
		else if (strcmp(token[i], ";") == 0)
		{
			++nomulti;
			for (int a = 0; a < i; a++)
			{
				sorttok[a] = token[a];
			}

			for (int a = 0; token[a + i + 1] != NULL; a++)
			{
				sorttok2[a] = token[a + i + 1];
				++endcheck;
			}
			sorttok[i] = NULL;   // 끝 체크
			sorttok2[endcheck] = NULL;
			int check;
			check = yesnopipe(sorttok);

			mulprom(sorttok2);
			break;
		}
		else if (strcmp(token[i], ",") == 0)
		{
			++nomulti;
			for (int a = 0; a < i; a++)
			{
				sorttok[a] = token[a];
			}

			for (int a = 0; token[a + i + 1] != NULL; a++)
			{
		
				sorttok2[a] = token[a + i + 1];
				++endcheck;
			}
			sorttok[i] = NULL;   // 끝 체크
			sorttok2[endcheck] = NULL;
			int check;
			check = yesnopipe(sorttok);

			mulprom(sorttok2);
			break;
		}
		
			

	}
	if (nomulti == 0)
	{
		yesnopipe(token);
	}
}
	int cd(char* token[], int num)
	{
		const char* target = NULL;
		int check;
		if (num > 3)
		{
			fprintf(stderr, "cd:too many 인자");
		}
		if (num == 1)
		{
			target = getenv("HOME");
			if (!target) //getenv 는 실패시 NULL 반환하니까
			{
				fprintf(stderr, "cd:이동 실패\n");
				return 0;
			}
		}
		else if (num == 2)
		{
			target = token[1];
		}
		else //인자가 3개인 경우  -> 예를들어 cd -L .   이런경우는 검색으로 알았습니다.
		{
			if (strcmp(token[1], "-L") == 0 || strcmp(token[1], "-P") == 0)
			{
				target = token[2];
		
			}
			else
			{
				fprintf(stderr, "cd: invalid option %s\n", token[1]);

			}
		}
		check = chdir(target);
		if (check == 0)
		{
			return 1;
		}
		else
		{
			fprintf(stderr, "cd error");
			return 0;
		}
		
	}
	int pwd(char* token[], int num)
	{
		if (num > 1)
		{
			fprintf(stderr, "pwd error");
			return 0;
		}
		else
		{
			prindir();
		}
	}

	void scan(void)   //스캔하고, 띄어쓰기 제거, 토큰으로 분류
	{

		char line[MAX];
		char* nospace[MAX];

		while (1)
		{
			if (!fgets(line, sizeof(line), stdin)) break;
			line[strcspn(line, "\n")] = NULL; // \n 제거
			addspace(line);
			int i = 0;
			char* token = strtok(line, " ");
			while (token != NULL)
			{
				nospace[i++] = token;
				token = strtok(NULL, " ");
			}
			nospace[i] = NULL;
			int bg = 0;
        if (i > 0 && strcmp(nospace[i-1], "&") == 0) {
            bg = 1;
            nospace[--i] = NULL;  // 마지막 "&" 토큰 제거
        }
        nospace[i] = NULL;
		if (i >0 && strcmp(nospace[0], "exit") == 0)   // exit 구현현
		{
			printf("logout");
			exit(0);
		}
        // 분기: bg면 백그라운드
        if (bg) 
		{
            goback(nospace);
		}
			else{
				mulprom(nospace);
				prindir();
				}
	}
}
	

	void prindir(void)
	{
		char dirarr[MAX];
		if (getcwd(dirarr, sizeof(dirarr)) != NULL)
		{
			printf("%s", dirarr);
		}
		else
		{
			printf("directory error");
		}
		printf("\n");
		printf("$");

	}

	int main(void)
	{
		prindir();
		scan();
	}