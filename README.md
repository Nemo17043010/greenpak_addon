# greenpak_addon
Armadilloでgreenpakを動かす

# 接点入力-接点出力の関係
[IC13：接点入力 IO0(PIN19)] - [J1:9pin IO6(PIN11)]

[IC14：接点入力 IO1(PIN18)] - [IC8:接点出力 IO10(PIN5)]

[IC15：接点入力 IO2(PIN17)] - [IC9:接点出力 IO11(PIN4)]

[IC16：接点入力 IO3(PIN16)] - [IC10:接点出力 IO12(PIN3)]

# カウンタの使い方

## カウンタ回路をリセットする 
i2cset -y 3 0x08 0x7a 0x00 b //CNT1のresetをアサート
i2cset -y 3 0x08 0x7a 0x80 b //CNT1のresetをデアサート

7C 999:992 CNT0(16-bit) Counted Value Q[7:0]　->上位16bitのカウンタ
7D 1007:1000 CNT0(16-bit) Counted Value Q[15:8] ->上位16bitのカウンタ
7E 1015:1008 CNT2(8-bit) Counted Value Q[7:0] ->下位8bitのカウンタ
