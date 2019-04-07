# 对rowid的理解
&emsp;&emsp;rowid是数据库的一个伪列，在建立表的时候Oracle会自动为每个表建立ROWID列。伪列实际上不存储在表中，可以从这些伪列中查询值，但不能插入、更新或删除。   
&emsp;&emsp;rowid是用来唯一标识一行记录的，是存储每条记录的实际物理地址。   

# 查询某条记录的rowid值

```
SQL> select ROWID from user_test where rownum<3;

ROWID
------------------
AAAQObAAFAAAACDAAA
AAAQObAAFAAAACFAAA

rowid的格式如下:
1. 数据对象编号: AAAQOb, data_obj#,即obj$
2. 相对文件编号: AAF, file#
3. 块编号: AAAACD, block#
4. 行编号: AAA, slot
```

# 计算具体的dataobj#, file#, block#, slot
![图片 1.png](https://i.loli.net/2019/04/07/5ca94f152b9b0.png)

```
	rowid是base64编码，用A-Z，a-z, 0-9,+,/共64个字符来表示，A表示0，B表示1等等。
	则：	
	data_obj#=AAAQOb=0*64^5 + 0*64^4 + 0*64^3 + 16*64^2 + 14*64^1 + 27*64^0 = 66459
	file#=AAF=0*64^2 + 0*64^1 + 5*64^0 = 5
	block#=AAAACD=2*64^1 + 3*64^0 = 128
	slot=AAA = 0
```

# 根据data_obj#, file#, block#, slot计算rowid
&emsp;&emsp;实际上就是将十进制数转化成64进制，当然从二进制转换比较简单。  
&emsp;&emsp;将二进制数从右往左，6位一组，然后将这6位二进制转成十进制，然后替换成base64的字符即可。  

```
例:
	dataobj#=66444=1 00000011 10001100 = 010000 001110 001100 = 16 14 12 = Q O M = AAAQOM(补齐6位)
	file#=1 = AAB
	block#=70321=1 00010010 10110001 = 010001 001010 110001 = 17 10 49 = R K x = AAARKx
	slot=0 = AAA
则:rowid=AAAQOMAABAAARKxAAA
```

# SQL查询出data_obj#,file#,block#,slot的值

```
SQL> select dbms_rowid.rowid_object(rowid) object_id,
  2  dbms_rowid.rowid_relative_fno(rowid) file_id,
  3  dbms_rowid.rowid_block_number(rowid) block_id,
  4  dbms_rowid.rowid_row_number(rowid) num 
  5  from user_test;

 OBJECT_ID    FILE_ID	BLOCK_ID	NUM
---------- ---------- ---------- ----------
     66459	    5	     131	  0
     66459	    5	     133	  0
```
#总结
&emsp;&emsp;根据data_obj#确定数据保存的数据段，根据file#确定数据所在的文件，根据block#确定数据保存在文件的哪里块，根据slot确定数据保存在块中的哪一行，因此根据rowid可以具体定位数据的位置。对数据库中记录行的最快检索操作就是通过rowid来进行查找的。