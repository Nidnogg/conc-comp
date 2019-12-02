/* Disciplina: Computacao Concorrente */
/* Prof.: Silvana Rossetto */
/* Laboratório: 10 */
/* Codigo: Acessando variável compartilhada em um programa multithreading Java */
/* -------------------------------------------------------------------*/

//classe da estrutura de dados (recurso) compartilhado entre as threads
class S {
  //recurso compartilhado
  int r;
  //construtor
  public S() { 
     this.r = 0; 
  }

/*

   public void inc() { 
     this.r++; 
  }

  public int get() { 
      return this.r; 
  }
*/
  // ou...
  
//Bloco Synchronized 
  public synchronized void inc() { 
     this.r++; 
  }

  public synchronized int get() { 
      return this.r; 
  }
}

//classe que estende Thread e implementa a tarefa de cada thread do programa 
class T extends Thread {
   //identificador da thread
   int id;
   //objeto compartilhado com outras threads
   S s;
  
   //construtor
   public T(int tid, S s) { 
      this.id = tid; 
      this.s = s;
   }

   //metodo main da thread
   public void run() {
      System.out.println("Thread " + this.id + " iniciou!");
      for (int i=0; i<10000000; i++) {
         this.s.inc();  
      }
      System.out.println("Thread " + this.id + " terminou!"); 
   }
}

//classe da aplicacao
class TIncrementoBase {
   static final int N = 2;

   public static void main (String[] args) {
      //reserva espaço para um vetor de threads
      Thread[] threads = new Thread[N];

      //cria uma instancia do recurso compartilhado entre as threads
      S s = new S();

      //cria as threads da aplicacao
      for (int i=0; i<threads.length; i++) {
         threads[i] = new T(i, s);
         threads[i].start();  //inicia threads
      }
      
      //espera pelo termino de todas as threads
      for (int i=0; i<threads.length; i++) {
         try { threads[i].join(); } catch (InterruptedException e) { return; }
      }

      System.out.println("Valor de s = " + s.get()); 
   }
}
