# Trabalho Final da disciplina Fundamentos de Computação Gráfica do semestre 2024/1 da UFRGS
## Leonardo Heisler e Thiago Gonçalves
### Contribuição de cada membro
#### Thiago Gonçalves
- Lógica do jogo (Classes dos alvos, criação do mapa, lógica de tiro ao alvo, etc)
- Lógica de colisões
- Movimentação (incluindo gravidade)
- Câmera em 3a pessoa

#### Leonardo Heisler
- Inclusão e renderização dos .obj
- Inclusão das texturas e shaders pros objetos e crosshair
- Movimentação dos targets (bezier)
- Iluminação dos objetos no mundo

### Uso de ferramentas de Inteligência Artificial
Usamos bastante o ChatGPT para auxílio na criação de funções, principalmente em sua correção. As vezes, quando criávamos funções não conseguíamos identificar o erro, e o ChatGPT arrumava a lógica em nosso código.
Achamos muito útil, nos ajudou principalmente em debugging e a nos familiarizar melhor com o OpenGL (uma tecnologia que não tínhamos domínio antes de fazer os laboratórios e iniciar o trabalho final). 
Contudo, nem sempre a ferramenta nos auxiliava, as vezes nos enviando códigos que não cumpriam o propósito original do programa, ou simplesmente não funcionavam.

### Processo de desenvolvimento
Inicialmente, usamos como base o laboratório 2. Porém, para adicionar objetos e iluminação, achamos mais útil utilizar o laboratório 4 e migrar nosso código. 
Na branch "main" do github, a maioria das mudanças foi feita pelo Thiago Gonçalves.
Em outras branches, a maioria das mudanças foi feita pelo Leonardo Heisler.
Nós conversávamos por WhatsApp para atualizar um ao outro sobre o código e para juntar nossos códigos.
Em cima disso começamos a implementar a lógica do jogo e tentar atendar a maior parte dos requisitos do trabalho.
Contudo, a aplicação de texturas foi um desafio e necessitamos pegar parte do fragment shader do laboratório 5 pra fazer o programa funcionar.

### Exemplos da aplicação
![image](https://github.com/user-attachments/assets/f782b4ac-a90d-4e3b-b6a7-bdb119d68525)
![image](https://github.com/user-attachments/assets/898fef75-1a9c-4de7-a669-e218da89ced6)

### Utilização da aplicação
- Utilize as teclas W,A,S,D para se movimentar.
- Utilize a tecla SPACE para pular.
- Clique com o botão esquerdo do mouse para atirar.
- Utilize a tecla SHIFT ESQUERDO pra andar mais devagar.
- Utilize a tecla L para alternar entre câmeras em 1a e 3a pessoa.
- Utilize a tecla R para resetar o jogo.
- Utilize a tecla F para alternar entre modo janela e fullscreen.
- Aperte ESC para fechar o jogo.

### Passos necessários pra compilar e executar
(Utilizamos o Visual Studio Code como nossa ferramenta de trabalho)
- Dependências: Cmake, makefiles, e seus plug-ins pro VS:Code (talvez opcional)
#### Para Windows
- No caso do Thiago, ele precisou usar o compilador do Visual Studio 2022 (sugestão caso não seja possível compilar)
1) Baixar o arquivo fonte
2) Abrir a pasta principal do código (TFinal-FCG) com o VS:Code
3) Compilar utilizando o Cmake ou makefiles (dentro da pasta do projeto, executar `cmake .`)
![image](https://github.com/user-attachments/assets/e3a7f57b-f1bc-45a8-83d9-3a1c7fe1e503)

#### Para Linux
1) Baixar o arquivo fonte
2) Entrar no diretório
3) Escrever `make run` no terminal 
