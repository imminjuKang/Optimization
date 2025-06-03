# Optimization

input.txt : 50개의 프로세스에 대한 입력 값

prgram.c : 초기 코드

program1.c : optimization 과정

program2.c : 최종 최적화 코드

1. linux 시스템인 ubuntu에서 gcc -Og -pg program.c -o program으로 컴파일

2. gprof program gmoun.out > result.txt로 gprof 결과 확인

3. flat profile과 call graph를 참고하여 최적화 진행
