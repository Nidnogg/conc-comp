/* Disciplina: Computacao Concorrente */
/* Prof.: Silvana Rossetto */
/* Laboratório: 11 */
/* Codigo: Produtor/consumidor em Java */
/* -------------------------------------------------------------------*/

// Monitor
class Buffer {
  static final int N = 10;    //tamanho do buffer
  private int[] buffer = new int[N];  //reserva espaco para o buffer 
  private int count=0, in=0, out=0;   //variaveis compartilhadas
  
  // Construtor
  Buffer() { 
    for (int i=0;i<N;i++)  buffer[i] = -1; 
  } // inicia o buffer

  // Insere um item
  public synchronized void Insere (int item) {
    try { 
      while(this.count == N) {
        this.wait();
      } 

      this.buffer[in] = item;
      this.in = (this.in + 1) % N;
      this.count++;

      System.out.println("Thread " + item + " (producer) has inserted " + item + "  into buffer. count = " + this.count + " in = " + this.in + " out = " + this.out);
      System.out.print("buffer [ ");
      for(int i = 0; i < N; i++) {
        System.out.print(this.buffer[i] + " ");
      }
      System.out.print("]\n");
      this.notifyAll();
    } catch (InterruptedException e) {
      System.out.println("Failed to insert to buffer\n");
    }
  }
  
  // Remove um item
  public synchronized int[] Remove (int item) {
   int[] aux = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  
   try {
     while(count < N) {
       this.wait();
     }
     
     for(int i = 0; i < N; i++) {
      aux[i] = this.buffer[out];
      this.buffer[out] = -1;
      out = (out + 1) % this.N;
      this.count--;
   
      System.out.println("Thread " + item + " (consumer) has removed " + aux + " from buffer. count = " + this.count + " in = " + this.in + " out = " + this.out);
      System.out.print("buffer [ ");
      for(int j = 0; j < N; j++) {
        System.out.print(this.buffer[j] + " ");
      }
      System.out.print("]\n");
    }

     this.notifyAll();
    } catch (InterruptedException e) { System.out.println("Failed to remove from buffer\n"); }
    return aux;
  }

}

//--------------------------------------------------------
// Consumidor
class Consumidor extends Thread {
  int id;
  int delay;
  Buffer buffer;

  // Construtor
  Consumidor (int id, int delayTime, Buffer b) {
    this.id = id;
    this.delay = delayTime;
    this.buffer = b;
  }

  // Método executado pela thread
  public void run () {
    int[] item;
    try {
      for (;;) {
        item = this.buffer.Remove(this.id);
        sleep(this.delay); //...simula o tempo para fazer algo com o item retirado
      }
    } catch (InterruptedException e) {       
      System.out.println("Failed to run\n");
    }
  }
}

//--------------------------------------------------------
// Produtor
class Produtor extends Thread {
  int id;
  int delay;
  Buffer buffer;

  // Construtor
  Produtor (int id, int delayTime, Buffer b) {
    this.id = id;
    this.delay = delayTime;
    this.buffer = b;
  }

  // Método executado pelo thread
  public void run () {
    try {
      for (;;) {
        this.buffer.Insere(this.id); //simplificacao: insere o proprio ID
        sleep(this.delay);
      }
    } catch (InterruptedException e) {System.out.println("Failed to insert into buffer!");}
  }
}

//--------------------------------------------------------
// Classe principal
class PC {
  static final int P = 5;
  static final int C = 5;

  public static void main (String[] args) {
    int i;
    Buffer buffer = new Buffer();      // Monitor
    Consumidor[] cons = new Consumidor[C];   // Consumidores
    Produtor[] prod = new Produtor[P];       // Produtores

    for (i=0; i<C; i++) {
       cons[i] = new Consumidor(i+1, 1000, buffer);
       cons[i].start(); 
    }
    for (i=0; i<P; i++) {
       prod[i] = new Produtor(i+1, 1000, buffer);
       prod[i].start(); 
    }

  }
}
