include ../common.mk

# ---- Assignment-specific settings ----
TARGET  = midterm
SRCS    = $(wildcard src/*.c)
OBJS    = $(SRCS:.c=.o)

PRESENTATION  = presentation/presentation.tex

.PHONY: all code pdf clean run

all: code pdf

code: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

pdf:
	$(LATEXMK) $(LATEXOPTS) -cd $(PRESENTATION)

run: code
	./$(TARGET)

clean:
	rm -f $(TARGET) $(OBJS)
	$(LATEXMK) -C -cd $(PRESENTATION)
