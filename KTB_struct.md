# op=0x01

```
KTB Redo 
op: 0x01  ver: 0x01 
compat bit: 4 (post-11) padding: 1
op: F  xid:  0x0002.01c.00000254    uba: 0x00c080f1.0060.0e

对应的结构:
typedef struct KTB{
	uint8_t op;   //0x01
	uint24_t unknown0;
	unit32_t unknown1;
	unit16_t xid0;
	uint16_t xid1;
	uint32_t xid2;
	uint32_t uba0;
	uint16_t uba1;
	uint8_t uba2;
	uint8_t unknown2;
}KTB;
```

# op=0x02
```
KTB Redo 
op: 0x02  ver: 0x01 
compat bit: 4 (post-11) padding: 1
op: C  uba: 0x00c0d5b7.0082.1d

对应的结构:
typedef struct KTB{
	uint8_t op;
	uint24_t unknown0;
	uint32_t unknown1;
	uint32_t uba0;
	uint16_t uba1;
	uint8_t uba2;
	uint8_t unknown2;
}KTB;
```

# op=0x03 & op=0x04
```
KTB Redo
op: 0x03  ver: 0x01 
compat bit: 4 (post-11) padding: 1
op:Z
```
```
KTB Redo
op: 0x04  ver: 0x01 
compat bit: 4 (post-11) padding: 1
op: L  itl: xid:  0x0007.00b.0000057d    uba: 0x00c004b7.007b.0b
			flg: C---	lkc: 0	scn: 0x0000.001868a7
```
```
op=0x03和op=0x04常出现在回滚事务中。
```

# op=0x05
```
KTB Redo 
op: 0x05  ver: 0x01  
compat bit: 4 (post-11) padding: 1
op: R  itc: 1
 Itl           Xid                  Uba         Flag  Lck        Scn/Fsc
0x01   0x0005.001.00000291  0x00c118ad.0066.01  -B--    1  fsc 0x0000.00000000
```

# op=0x11
```
KTB Redo 
op: 0x11  ver: 0x01 
compat bit: 4 (post-11) padding: 1
op: F  xid:  0x0003.00e.00000245    uba: 0x00c0d5b7.0082.1c
Block cleanout record, scn:  0x0000.000a813e ver: 0x01 opt: 0x02, entries follow...
  itli: 1  flg: 2  scn: 0x0000.000a80e5
  itli: 2  flg: 2  scn: 0x0000.000a8059

对应的结构:
Block cleanout record开始是块清除信息,之前的信息跟op=0x01时相同,块清除信息个人觉得用不上。
```
# op=0x12
```
KTB Redo
op: 0x12  ver: 0x01  
compat bit: 4 (post-11) padding: 1
op: C  uba: 0x00c0f5f8.0060.32
Block cleanout record, scn:  0x0000.000ad642 ver: 0x01 opt: 0x02, entries follow...
  itli: 1  flg: 2  scn: 0x0000.000ad642

对应的结构:
Block cleanout record之前的结构跟op=0x02的相同。
```
