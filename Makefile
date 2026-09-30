NAME	= codexion

CC		= cc
CFLAGS	= -Wall -Wextra -Werror -pthread
INCL	= -I includes

SRC_DIR	= src
OBJ_DIR	= obj

SRCS	= main.c \
		  parsing.c \
		  utils.c \
		  time_log.c \
		  init.c \
		  destroy.c \
		  coder.c \
		  threads.c \
		  dongle.c \
		  acquire.c \
		  heap_utils.c \
		  heap.c \
		  monitor.c

OBJS	= $(addprefix $(OBJ_DIR)/, $(SRCS:.c=.o))
HEADER	= includes/codexion.h

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HEADER)
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCL) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re