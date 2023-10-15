# greenpak_addon
Armadilloでgreenpakを動かす

# SLG46824_6_pulse_cnt_add.gp6 の仕様

* 接点入力-接点出力の連動 3ポート
* 以下のGPIOピンを使用した24bitパルスカウンター 1 ポート
  * カウンタの入力：IO13(PIN2)　下段の左から1番目
  * カウンタの出力：IO14(PIN1)　下段の左から3番目

## 接点入力-接点出力の関係


[IC14：接点入力 IO1(PIN18)] - [IC8:接点出力 IO10(PIN5)]

[IC15：接点入力 IO2(PIN17)] - [IC9:接点出力 IO11(PIN4)]

[IC16：接点入力 IO3(PIN16)] - [IC10:接点出力 IO12(PIN3)]


## カウンタの使い方

* カウンタの入力：IO13(PIN2)　下段の左から1番目
* カウンタの出力：IO14(PIN1)　下段の左から3番目

### カウンタ閾値の設定

A5 1327:1320 REG_CNT0_Data[7:0] Data[7:0]  ->上位16bitのカウンタ

A6 1335:1328 REG_CNT0_Data[15:8] Data[15:8] ->上位16bitのカウンタ

AF 1407:1400 REG_CNT2_Data[7:0] Data[7:0] -> 下位8bitでのカウンタ閾値


### カウンタ回路をリセットする 
i2cset -y 3 0x08 0x7a 0x00 b //CNT1のresetをアサート

i2cset -y 3 0x08 0x7a 0x80 b //CNT1のresetをデアサート

### カウンタの確認方法

以下のレジスタが1パルス入力される毎に1ずつ減っていく。

7C 999:992 CNT0(16-bit) Counted Value Q[7:0]　->上位16bitのカウンタ

7D 1007:1000 CNT0(16-bit) Counted Value Q[15:8] ->上位16bitのカウンタ

7E 1015:1008 CNT2(8-bit) Counted Value Q[7:0] ->下位8bitのカウンタ

* 7Eレジスタが00のとき(reset直後)
* 7C,7Dレジスタが00で7Eレジスタが00のとき(上位16bitに桁上がりするとき)

の時には7E,または7C,7Dレジスタをffにする処理で1パルス使用する


# SLG468246_sample.gp6 の仕様

* 接点入力-接点出力の連動 1ポート
* 以下の接点入力ピンを使用した24bitパルスカウンター　1ポート
  * カウンタの入力：IC14：接点入力 IO1(PIN18)　
  * カウンタの出力：IC8:接点出力 IO10(PIN5)
* アナログコンパレーター　1ポート


## 接点入力-接点出力の関係
[IC13：接点入力 IO0(PIN19)] - [J1:9pin IO6(PIN11)]

[IC16：接点入力 IO3(PIN16)] - [IC10:接点出力 IO12(PIN3)]

## カウンタの使い方

* カウンタの入力：IC14：接点入力 IO1(PIN18)　
* カウンタの出力：IC8:接点出力 IO10(PIN5)
  
## カウンタ回路をリセットする 
i2cset -y 3 0x08 0x7a 0x00 b //CNT1のresetをアサート

i2cset -y 3 0x08 0x7a 0x80 b //CNT1のresetをデアサート

## カウンタの確認方法

以下のレジスタが1パルス入力される毎に1ずつ減っていく。

7C 999:992 CNT0(16-bit) Counted Value Q[7:0]　->上位16bitのカウンタ

7D 1007:1000 CNT0(16-bit) Counted Value Q[15:8] ->上位16bitのカウンタ

7E 1015:1008 CNT2(8-bit) Counted Value Q[7:0] ->中位8bitのカウンタ

7F 1023:1016 CNT4(8-bit) Counted Value Q[7:0] -> 下位8bitのカウンタ

* 7Fレジスタが00のとき(reset直後)
* 7Eレジスタが00で7Fレジスタが00のとき(中位8bitに桁上がりするとき)
* 7C,7Dレジスタが00で7Eレジスタが00のとき(上位16bitに桁上がりするとき)

の時には7E,または7C,7Dレジスタをffにする処理で1パルス使用する

  ## アナログコンパレータ

* 入力：IO13(PIN2)　下段の左から1番目
* 出力：IC9:接点出力 IO11(PIN4)

## 閾値の設定

- 8A [1105:1104] ACMP1H Gain divider　, [1111:1106] ACMP1H Vref 
   - ACMP1H Gain divider:  00: 1x , 01: 0.5x , 10: 0.33x , 11: 0.25x
   - ACMP Vref select: 000000: 32 mV ~ 111110: 2.016 V/step = 32 mV; 111111: External Vref
     
閾値 =　[ACMP1H Gain divider] × [ACMP Vref ]
  

