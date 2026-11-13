# Makefile taken from https://github.com/arfc/moltres/blob/devel/paper/Makefile

NAME=writeup

all: $(NAME).pdf

$(NAME).pdf : $(NAME).md 
	pandoc  $(NAME).md -o $(NAME).pdf

clean :
	rm $(NAME).pdf
