
# 经验之谈
&emsp;&emsp;如果是想最后能构建出SQL语句，应该按以下步骤：  
&emsp;&emsp;1. redo log文件的结构能区分，即计算redo record，从redo record中计算出change，通过大量的archive log文件做大量的测试。  
&emsp;&emsp;2. 计算每个change，先能构建简单的增删改，这里先不要考虑事务。  
&emsp;&emsp;3. 分析事务，这里需要判断事务的commit和rollbak，构建的话只需要构建commit的事务。  
&emsp;&emsp;4. 之后也可以继续学习构建DDL语句，也可以尝试做些小demo，例如保持两台oracle的数据一致，可以通过解析主机的redo log，将sql传到备机执行。

# 前言
&emsp;&emsp;要从oracle redo日志中反编译出sql，有以下3个步骤：  
&emsp;&emsp;1. 从redo log中将redo block header、redo record、redo record header、redo change、redo change header分离出来。  
&emsp;&emsp;2. 根据每一个redo change中的opcode确定change的操作类型。  
&emsp;&emsp;3. 根据xid区分事务。  
![](https://i.loli.net/2019/03/22/5c94d32f868d2.png)

# redo文件结构
## block 0的数据格式
&emsp;&emsp;block 0是redo的第一个数据块，记录了块大小，块总数等信息。需要注意的是，block不包含redo block header（即块头），不包含在块总数中。  

```
typedef struct file_header_0{
	uint32_t unknown0[5];
	unit32_t blocksize;  //块大小,512/1024...
	unit32_t blockcount;  //当前文件的总块数
	unit32_t unknown1[2];
	unit32_t zero[119];
}Redo_fh0
```

## block1的数据格式
&emsp;&emsp;第2块做为数据库头，包含数据库信息(如版本号、数据库ID、文件序号等)。  

```
tyoedef struct redo_block_header{
	uint32_t signature;  //签名
	unit32_t blocknum;   //块号
	unit32_t sequence;   //顺序号
	unit16_t offset;     //当前块的第一个redo record开始的位置，最高位舍弃
	unit16_t checksum;   //块的checksum,写入时更新
}Redo_bh

typedef struct RBA{  //redo文件中的位置信息
	unit32_t sequence;
	unit32_t blocknum;
	unit16_t offset;
}Redo_RBA

typedef struct scn{
	unit32_t scnbase;
	unit16_t scnwrapper;
}Redo_scn

typedef struct file_header_1{
	Redo_bh blockheader;
	unit32_t unknown0;
	unit32_t Compatibility Vsn;
	unit32_t db id;
	unit64_t db name;  //猜测用来表示数据库名称
	unit32_t control seq;
	unit32_t file size;
	unit32_t blksize;
	unit16_t file number;
	unit16_t file type;
	unit32_t activation id;
	unit8_t zero[36];
	unit8_t unknown1[64];
	unit32_t nab;  //当前文件最后一个有真实记录块的下一个块
	unit32_t resetlogs count;
	Redo_scn resetlos scn;
	unit16_t 0;
	unit32_t hws;
	unit32_t thread;
	Redo_scn low scn;
	unit16_t 0;
	unit32_t low scn time;
	Redo_scn next scn;
	unit16_t 0;
	unit32_t nex scn time;
	unit32_t unknown2;
	Redo_scn enabled scn;
	unit16_t 0;
	unit32_t enabled scn time;
	Redo_scn thread closed scn;
	unit16_t 0;
	unit32_t thread closed scn time;
	unit8_t unknown3[52];
	Redo_scn prev resetlogs scn;
	unit16_t 0;
	unit32_t prev resetlogs count;
	unit8_t unknown4[216]
}Redo_fh1
```

## block2的数据格式
&emsp;&emsp;从第三块(block 2)开始，块里面存储着Oracle的redo日志。由块头和块体构成，其中块头即结构体Redo_bh，和block1的块头结构一致。  

```
tyoedef struct redo_block_header{
	uint32_t signature;  //签名
	unit32_t blocknum;   //块号
	unit32_t sequence;   //顺序号
	unit16_t offset;     //当前块的第一个redo record开始的位置，最高位舍弃
	unit16_t checksum;   //块的checksum,写入时更新
}Redo_bh

typedef struct block{
	Redo_bh blockheader;
	unit8_t redo record[496];  //redo record，日志记录
}
```
&emsp;&emsp;块头分析：  
&emsp;&emsp;1. signature -- 表示这是一个redo block。  
&emsp;&emsp;2. block number -- 当前块的编号。  
&emsp;&emsp;3. sequence -- 顺序号(序列号)，即v$log视图的SEQUENCE#字段  
&emsp;&emsp;4. offset -- 标记本快中第一个redo record开始的位置，位置是包括块头的，且需要过滤最高位。   
&emsp;&emsp;5. checksum -- 校验值，块写入时计算得到。  

&emsp;&emsp;操作记录(redo record)的结构:  
&emsp;&emsp;1. 一个记录头(redo record header)  
&emsp;&emsp;2. 多个change。   
### redo record header
&emsp;&emsp;record header的长度有24字节和68字节两种情况。

```
typedef struct record_header0{
	unit4_t len;  //redo record的长度
	unit8_t vld;
	unit8_t unknown0;
	unit16_t record header scn wrapper;
	unit32_t record header scn base;
	unit16_t subscn; 
}

typedef struct record_header1{
	unit8_t unknown2[10]:
}Redo_rh24  //长度为24字节的record header结构

typedef struct record_header2{
	unit8_t unknown0[50];
	unit32_t timestamp;  //本次操作的时间戳
}Redo_rh68  //长度为68字节的record header结构
```
&emsp;&emsp;**如何确定record header的长度是24字节还是68字节?**  
&emsp;&emsp;经过多次测试以及查找大神的文档，得出一个结论，且这个结论至今还未出错。在一位大神的文档中看到说record header的长度由vld决定，在11G版本下，我测试发现，**只要vld的值中包括了4，长度就是68字节，反之是24字节**。包括4的意思是，假设vld是12，4+8=12，说明vld包括了4，所以长度是68字节。vld的取值由以下表格确定：  

| Mnemonic  | Value | Description                      |  
|-----------|-------|----------------------------------|  
| KCRVOID   |  0    | The contents are not valid.      |
| KCRVALID  |  1    | Includes change vectors          |
| KCRDEPND  |  2    | Includes commit SCN              |
| KCRVOID   |  4    | Includes dependent SCN           |
| KCRNMARK  |  8    | New SCN mark record. SCN allocated exactly at this point in the redo log by this instance  
| KCRMARK   |  16   | Old SCN mark record. SCN allocated at or before this point in the redo. May be allocated by another instance  
| KCRORDER  |  32   | New SCN was allocated to ensure redo for some block would be ordered by inc/seq# when redo sorted by SCN  

&emsp;&emsp;表格的含义我不太清楚，得出的结论是我多次验证的来的。  
### redo change
&emsp;&emsp;记录头之后就是一组redo change了，一个操作对应一个change，比如插入，更新，块清除等。每个change对应一个操作码(opcode)。  
&emsp;&emsp;每个change又分为change header，change length list(change向量表)，和change body。 
#### redo change header 

```
typedef struct opcode{
	unit8_t layer number;  
	unit8_t code;  
}Redo_opcode
typedef struct change header{
	Redo_opcode opcode;
	unit16_t CLS;
	unit16_t AFN;
	unit16_t OBJ0;
	unit32_t DBA;
	unit32_t change header base;
	unit16_t change header wrapper;
	unit16_t unknown0;
	unit8_t SEQ;
	unit8_t TYP;
	unit16_t OBJ1;
}Redo_ch  //长度固定为24字节
```

&emsp;&emsp;计算OBJ(**这里的OBJ是data\_object\_id,不是object\_id**)：OBJ1 | (OBJ0 << 16)  

```
例如：OBJ0=0x0001, OBJ1=0x0388  
     OBJ0左移16位:00000000 00000001 00000000 00000000
     OBJ1:00000011 10001000
     或的结果:00000000 00000001 00000011 10001000
     转成十进制:66440
OBJ为本次操作的表的data_object_id,可以根据OBJ从数据字典中查到表名，字段等信息用来构建SQL语句。
```

#### redo change length list
&emsp;&emsp;redo change header后是一个change向量表，主要是用来计算change的长度。**change向量表的长度按4字节对齐，然后内部是每2字节一段。第1，2字节表示向量表总长度(未进行4字节对齐之前的有效长度)，总长度之后剩余的长度(有效总长度-2)，分为每2字节一段，每一段表示一个change部分的长度**，这些长度会按顺序写在向量表之后，向量表中的长度为change部分的有效长度。  
&emsp;&emsp;向量表是变长的，和redo record一样，长度在最开始，因此每次都应该先读取表示有效长度的2字节进行解析。  

```
例：change length list: 0a004800 31000800 0100c000
	1. 向量表总长度:0x000a = 10，有效长度是10，补齐4字节后是12，因此最后的2字节c000是无效字节
	2. 有效长度减去1,2字节后剩8字节，每2字节为一段，说明这个change被分成了4部分
	3. 第一部分有效长度: 0x0048 = 72, 第二部分有效长度: 0x0031 = 49, 第三部分有效长度: 0x0008 = 8, 第四部分有效长度: 0x0001 = 1
	4. 第一部分实际长度: 72字节, 第二部分实际长度: 52字节, 第三部分实际长度: 8字节, 第四部分实际长度: 4字节
	5. 整个change的实际长度: 24(header) + 12(length list) + 72 + 52 + 8 + 4 = 172字节
```

# 计算

![](https://i.loli.net/2019/03/23/5c95cbb2d3a9b.png)
![](https://i.loli.net/2019/03/23/5c95cbd9c0583.png)

```
[注意]本机是小端机器，高字节在前，低字节在后。
Block2:
	1. offset:0x8010,过滤最高位，0x0010=16,表示Block2的第一个redo record从16字节开始。
	2. record len:0x00000100 = 256，说明record的长度为256.
	3. vld:在record的偏移量是4字节，vld本身的长度是1字节，从Block2中可以得到vld=0x05=5(1+4)，说明当前record的record header长度为68字节。
	4. 68字节后，是record的第一个change[即6行的05021900处],change header固定为24字节。
	5. 24字节后，是change length list，即:04002000,当前change只有一个部分，长度为0x0020=32字节。
	6. 计算得到，第一个change的长度为:24(change header)+4(length list)+32=60字节。
	7. 从第6行的05021900处开始数60字节，接着的是第二个change,即8行的05011a00处。
	8. 24字节后是第二个change的length list:06001400 4c000000,length list的有效长度是6字节,当前change被分成了两部分，第一部分长度0x0014=20字节,第二部分长度=0x004c=76字节。
	9. 计算得到,第二个change的长度为:24+8(补齐4字节的倍数)+20+76=128字节。
	10. 到这里,第一个record结束了,接着是第二个record。
	11. 第二个record，从12行的3c000000处开始,这里回到第2步,从计算record len开始,同理。

Block3:
	1. 0ffset:0x8040,过滤最高位0x0040=64,说明Block3的第一个record开始于64字节处，而16字节-64字节之间的内容属于上一个Block的跨块record记录。
	2. 往后的计算，同Block2.

```




