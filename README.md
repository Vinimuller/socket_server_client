# socket_server_client

    Para a primeira vez rodando o código, abra um terminal e rode:
- make clean
- make

    Para iniciar o chat, primeiro inicie o server o comando:
make run-server

    Então, rode o client em outro(s) terminal(is) com o comando:
make run-client

    Os usuários e senhas são os seguintes:

| user   | password |
|--------|----------|
| marcio | 1234     |
| paula  | abcd     |
| vini   | pass     |

* Os comandos LISTA e ENVIAR são case-sensitiv, então é necessário utilizá-los com letras maiúsculas
** Para enviar uma mensagem ao outro usuário, a forma correta de texto é: ENVIAR marcio oi (ENVIAR [usuário] [mensagem])
*** Para encerrar a sessão de algum usuário, basta fechar o terminal do mesmo e os outros usuários saberão que este se desconectou.