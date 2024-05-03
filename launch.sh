#!/bin/bash

	trap "" SIGINT SIGTSTP SIGQUIT SIGKILL SIGABRT SIGTERM

	read_credenentials() {
		echo "  Введите ip-адрес сервера для подключения:"
		read server_hostname
		echo "  Введите номер порта сервера для подключения:"
		read server_port_no
	}

	launch_server() {
		trap "rm -rf /tmp/._ ; sleep 2" SIGINT SIGTSTP SIGQUIT
		/tmp/._/__server__ "$@"
		exit $?
	}

	launch_client() {
		/tmp/._/__client__ "$@"
		exit $?
	}

	case $1 in
	-u)
		export option=5
		;;
	--run-server)
		launch_server "${@:2}"
		;;
	--run-client)
		launch_client "${@:2}"
		;;
	*)

		printf "
    |-----------------------------------------------------------------------|
    |                      \033[48;2;255;0;0m\033[1;94m\033[38;2;255;255;255mСистема удаленного управления\033[0m                    |
    |-----------------------------------------------------------------------|

\033[1;94m\033[38;2;0;255;0m
  [1] Загрузить клиент
  [2] Загрузить сервер\033[0m

  Ваш выбор >>> "
		read option
		;;
	esac

	case $option in
	1) #КЛИЕНТ
		read_credenentials
		echo "  Вы хотите, чтобы другие пользователи получили доступ к вашей оболочке? [y/n] "
		read shell_permit
		case $shell_permit in
		n)
			launch_client $server_hostname $server_port_no
			;;
		y)
			launch_client $server_hostname $server_port_no --permit-shell-access
			;;
		*)
			echo "INVALID"
			exit -3
			;;
		esac
		;;
	2) # СЕРВЕР
		echo "  Введите номер порта сервера для подключения: "
		read server_port_no
		launch_server $server_port_no #НОМЕР ПОРТА ЧЕРЕЗ ARGV
		;;
	*)
		echo "  INVALID OPTION!!!"
		exit -1
		;;
	esac
