CC      = g++
PROF    = -ggdb
NOCRYPT =

C_FLAGS =  -Wall $(PROF) $(NOCRYPT) -Wno-write-strings -Wno-format-truncation -Wno-format-overflow -Wno-address -Wno-unused-but-set-variable -Wno-misleading-indentation -Wno-register -Wno-sequence-point -Wno-format-security
L_FLAGS =  $(PROF)
LIBS	= -lcrypt -lz
TARGET	= shadow

SRC_FILES := $(wildcard *.c)
O_FILES := $(patsubst %.c, o/%.o, $(SRC_FILES))

shadow: $(O_FILES)
	rm -f $(TARGET)
	$(CC) $(L_FLAGS) -o $(TARGET) $(O_FILES) $(LIBS)

o/%.o: %.c merc.h
	$(CC) -c $(C_FLAGS) $< -o $@


clean:
	rm -f $(O_FILES) $(TARGET)
	rm -f *.~
#	rm -f "#*#"

bzip:
	rm -f $(O_FILES)
	rm -f *.~
#	rm -f "#*#"
	#rm -f $(TARGET)
	tar -jcvf ../../$(TARGET).tar.bz2 ../../shadow

gzip:	
	rm -f $(O_FILES)
	rm -f *.~
#	rm -f "#*#"
	#rm -f $(TARGET)
	tar -zcvf ../../$(TARGET).tar.gz -"Xexclude.dat" ../


	
