NAME :=		a.out
SRCDIR :=	src/
INCLUDE :=	include/

CXX_SRC :=	main.cpp \
		Server.cpp \
		XdgShell.cpp \
		Seat.cpp \
		View.cpp \
		ServerCursor.cpp \
		ServerInput.cpp \
		ServerOutput.cpp

C_SRC :=	xdg-shell-protocol.c

CXX :=		g++
CXXFLAGS :=	-W -Wall -Wextra -g -std=c++17 \
		-lwayland-server -lwlroots -lxkbcommon \
		-DWLR_USE_UNSTABLE

CXX_SRC :=	$(addprefix $(SRCDIR), $(CXX_SRC))
C_SRC :=	$(addprefix $(SRCDIR), $(C_SRC))
CXX_OBJ :=	$(CXX_SRC:.cpp=.o)
C_OBJ :=	$(C_SRC:.c=.o)
RM :=		rm -f

DEFAULT :=	"\033[00;0m"
GREEN :=	"\033[0;32;1m"
RED :=		"\033[0;31;1m"
CYAN :=		"\033[0;36;1m"

all: $(NAME)

$(NAME): $(CXX_OBJ) $(C_OBJ)
	$(CXX) -o $(NAME) $(CXX_OBJ) $(C_OBJ) $(CXXFLAGS) && \
		echo -e $(GREEN)"[BIN]"$(CYAN) $(NAME)$(DEFAULT) || \
		echo -e $(RED)"[XX]"$(DEFAULT) $(NAME)
	for file in $(shell find . | cut -c 3- | grep -P ".*\.(cpp|hpp|c|h)"); \
		do fgrep -niH -e TODO -e FIXME $$file --color=auto; done; true

clean:
	echo -e $(CYAN)"Cleaning $(NAME) tmp files..." $(DEFAULT)
	$(RM) $(C_OBJ) $(CXX_OBJ)

fclean:	clean
	echo -e $(CYAN)"Cleaning $(NAME) executable..." $(DEFAULT)
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re

.SILENT: all $(NAME) clean fclean re

%.o: %.cpp
	@$(CXX) -c $< -o $@ $(CXXFLAGS) $(foreach dir, $(INCLUDE), -I$(dir)) && \
		echo -e $(GREEN)"[OK]"$(DEFAULT) $< || \
		echo -e $(RED)"[KO]"$(DEFAULT) $<
