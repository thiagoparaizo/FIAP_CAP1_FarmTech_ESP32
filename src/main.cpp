/*
 * FarmTech Solutions - Sistema de Sensores Agrícolas
 * 
 * Este código implementa um sistema de monitoramento de solo e irrigação automatizada
 * usando ESP32 e sensores simulados no ambiente Wokwi.
 * 
 * Sensores implementados:
 * - Sensor de Fósforo (P): Simulado por botão físico
 * - Sensor de Potássio (K): Simulado por botão físico
 * - Sensor de pH: Simulado com valores que variam de forma senoidal
 * - Sensor de umidade do solo: DHT22 (usado para simular sensor de umidade do solo)
 * 
 * Atuadores:
 * - Relé para controle de bomba de irrigação com LED indicador
 */

 #include <Arduino.h>
 #include "DHT.h"
 
 // Definição dos pinos - Ajustado para ESP32 DevKit C
 #define PIN_FOSFORO_BTN 12  // Botão simulando sensor de fósforo (P)
 #define PIN_POTASSIO_BTN 14 // Botão simulando sensor de potássio (K)
 #define PIN_PH_LDR  35      // LDR simulando sensor de pH (não usado agora)
 #define PIN_UMIDADE_DHT 15  // DHT22 simulando sensor de umidade do solo
 #define PIN_RELE 27         // Relé para controle da bomba de irrigação
 #define PIN_LED_BOMBA 2     // LED embutido para indicar estado da bomba
 
 // Configuração do sensor DHT
 #define DHTTYPE DHT22
 DHT dht(PIN_UMIDADE_DHT, DHTTYPE);
 
 // Limites para controle de irrigação
 const float LIMITE_UMIDADE_MIN = 30.0; // Umidade mínima para ativar irrigação (%)
 const float LIMITE_UMIDADE_MAX = 70.0; // Umidade máxima para desativar irrigação (%)
 const float LIMITE_PH_MIN = 6.0;       // pH mínimo ideal para a maioria das culturas
 const float LIMITE_PH_MAX = 7.5;       // pH máximo ideal para a maioria das culturas
 
 // Variáveis para controle de estado
 bool irrigacaoAtiva = false;
 bool fosforo_presente = false;
 bool potassio_presente = false;
 float ph_valor = 7.0;
 float umidade_solo = 50.0;
 
 // Estado anterior dos botões para detecção de mudança
 bool fosforo_anterior = false;
 bool potassio_anterior = false;
 
 // Variáveis para simulação do pH
 float ph_base = 7.0;           // pH base (neutro)
 float ph_amplitude = 3.5;      // Amplitude da variação (3.5 permite pH de 3.5 a 10.5)
 float ph_frequencia = 0.001;   // Frequência da oscilação (mais baixa = mudança mais lenta)
 unsigned long inicio_simulacao = 0;
 
 // Controle de tempo para leituras e envio de dados
 unsigned long ultimaLeitura = 0;
 const unsigned long INTERVALO_LEITURA = 5000; // 5 segundos
 
 void setup() {
   // Inicializa comunicação serial
   Serial.begin(115200);
   Serial.println("FarmTech Solutions - Sistema de Sensores Agrícolas");
   
   // Configura pinos - Modificado para conexão direta
   pinMode(PIN_FOSFORO_BTN, INPUT); // Sem pull-up interno
   pinMode(PIN_POTASSIO_BTN, INPUT); // Sem pull-up interno
   pinMode(PIN_PH_LDR, INPUT); // LDR (não usado para pH agora)
   pinMode(PIN_RELE, OUTPUT); // Relé da bomba
   pinMode(PIN_LED_BOMBA, OUTPUT); // LED indicador
   
   // Teste inicial dos dispositivos de saída
   Serial.println("Teste de inicialização: LED e relé ligados por 3 segundos");
   digitalWrite(PIN_RELE, HIGH);
   digitalWrite(PIN_LED_BOMBA, HIGH);
   delay(3000);
   digitalWrite(PIN_RELE, LOW);
   digitalWrite(PIN_LED_BOMBA, LOW);
   Serial.println("Teste concluído. Iniciando operação normal.");
   
   // Inicializa sensor DHT
   dht.begin();
   
   // Inicializa o tempo de simulação
   inicio_simulacao = millis();
   
   // Cabeçalho para saída CSV no Serial Monitor
   Serial.println("timestamp,fosforo,potassio,ph,umidade,irrigacao");
   
   Serial.println("Simulação de pH iniciada - valores variarão entre 3.5 e 10.5");
   Serial.println("pH ideal para irrigação: 6.0 - 7.5");
 }
 
 // Função para simular pH com variação senoidal
 float simularPH() {
   unsigned long tempo_atual = millis();
   unsigned long tempo_decorrido = tempo_atual - inicio_simulacao;
   
   // Calcular o pH usando uma função seno para criar uma variação suave
   // A função seno varia entre -1 e 1, multiplicamos pela amplitude e somamos à base
   float angulo = tempo_decorrido * ph_frequencia;
   float ph_simulado = ph_base + ph_amplitude * sin(angulo);
   
   // Garantir que o pH esteja dentro de limites realistas (0-14)
   ph_simulado = constrain(ph_simulado, 0.0, 14.0);
   
   // Arredondar para duas casas decimais
   return round(ph_simulado * 100) / 100.0;
 }
 
 // Função para verificar condições e decidir sobre irrigação
 void atualizarIrrigacao() {
   // Exibir diagnóstico de condições
   //Serial.println("----- Diagnóstico de Irrigação -----");
   //Serial.print("Umidade do solo: ");
   //Serial.print(umidade_solo);
   //Serial.print("% (Limite mínimo: ");
   //Serial.print(LIMITE_UMIDADE_MIN);
   //Serial.print("%, Limite máximo: ");
   //Serial.print(LIMITE_UMIDADE_MAX);
   //Serial.println("%)");
   
   //Serial.print("pH do solo: ");
   ///Serial.print(ph_valor);
   //Serial.print(" (Limite mínimo: ");
   //Serial.print(LIMITE_PH_MIN);
   //Serial.print(", Limite máximo: ");
   //Serial.print(LIMITE_PH_MAX);
   //Serial.println(")");
   
   // Verificar condição de umidade
   bool umidade_baixa = umidade_solo < LIMITE_UMIDADE_MIN;
   //Serial.print("Umidade baixa? ");
   //Serial.println(umidade_baixa ? "SIM" : "NÃO");
   
   // Verificar condição de pH
   bool ph_adequado = (ph_valor >= LIMITE_PH_MIN && ph_valor <= LIMITE_PH_MAX);
   //Serial.print("pH adequado? ");
   //Serial.println(ph_adequado ? "SIM" : "NÃO");
   
   // Classificar o pH para melhor entendimento
   String classificacao_ph;
   if (ph_valor < 6.0) {
     classificacao_ph = "ÁCIDO";
   } else if (ph_valor > 7.5) {
     classificacao_ph = "BÁSICO";
   } else {
     classificacao_ph = "IDEAL";
   }
   //Serial.print("Classificação do pH: ");
   //Serial.println(classificacao_ph);
   
   // Lógica de decisão para irrigação
   if (umidade_baixa && ph_adequado) {
     // Condições ideais para irrigar: solo seco e pH adequado
     irrigacaoAtiva = true;
     //Serial.println(">>> CONDIÇÕES SATISFEITAS: Ativando irrigação! <<<");
   } 
   else if (umidade_solo > LIMITE_UMIDADE_MAX || !ph_adequado) {
     // Desativa irrigação: solo úmido suficiente OU pH inadequado
     irrigacaoAtiva = false;
     
     if (umidade_solo > LIMITE_UMIDADE_MAX) {
       //Serial.println(">>> CONDIÇÃO INSATISFEITA: Umidade acima do limite máximo! <<<");
     }
     if (!ph_adequado) {
       //Serial.println(">>> CONDIÇÃO INSATISFEITA: pH fora dos limites aceitáveis! <<<");
     }
   }
   
   // Controla o relé e o LED baseado na decisão
   digitalWrite(PIN_RELE, irrigacaoAtiva ? HIGH : LOW);
   digitalWrite(PIN_LED_BOMBA, irrigacaoAtiva ? HIGH : LOW);
   
   //Serial.print("Estado da irrigação: ");
   //Serial.println(irrigacaoAtiva ? "ATIVADA" : "DESATIVADA");
   //Serial.print("Estado do pino do relé (pino ");
   //Serial.print(PIN_RELE);
   //Serial.print("): ");
   //Serial.println(digitalRead(PIN_RELE) ? "HIGH" : "LOW");
   //Serial.print("Estado do pino do LED (pino ");
   //Serial.print(PIN_LED_BOMBA);
   //Serial.print("): ");
   //Serial.println(digitalRead(PIN_LED_BOMBA) ? "HIGH" : "LOW");
   //Serial.println("------------------------------------");
 }
 
 // Função para simular valores de umidade para teste
 float simularUmidadeTeste() {
   // Simular valores que alternam entre alto e baixo para testar a irrigação
   static unsigned long ultimaMudanca = 0;
   static bool umidadeBaixa = false;
   
   unsigned long agora = millis();
   if (agora - ultimaMudanca > 25000) { // Alternar a cada 25 segundos
     ultimaMudanca = agora;
     umidadeBaixa = !umidadeBaixa;
     //Serial.print(">>> Simulação: mudando umidade para ");
     //Serial.print(umidadeBaixa ? "BAIXA" : "ALTA");
     //Serial.println(" <<<");
   }
   
   return umidadeBaixa ? random(15, 29) : random(71, 85);
 }
 
 void loop() {
   // Verificar os botões continuamente
   bool fosforo_atual = !digitalRead(PIN_FOSFORO_BTN);
   bool potassio_atual = !digitalRead(PIN_POTASSIO_BTN);
   
   // Detectar mudanças nos botões
   if (fosforo_atual != fosforo_anterior) {
     fosforo_anterior = fosforo_atual;
     fosforo_presente = fosforo_atual;
     //Serial.print(">>> Alteração no sensor de Fósforo: ");
     //Serial.print(fosforo_presente ? "Presente" : "Ausente");
     //Serial.println(" <<<");
   }
   
   if (potassio_atual != potassio_anterior) {
     potassio_anterior = potassio_atual;
     potassio_presente = potassio_atual;
     //Serial.print(">>> Alteração no sensor de Potássio: ");
     //Serial.print(potassio_presente ? "Presente" : "Ausente");
     //Serial.println(" <<<");
   }
   
   unsigned long tempoAtual = millis();
   
   // Lê os outros sensores e atualiza o estado a cada intervalo definido
   if (tempoAtual - ultimaLeitura >= INTERVALO_LEITURA) {
     ultimaLeitura = tempoAtual;
     
     // Simular o valor do pH
     ph_valor = simularPH();
     
     // Debug do pH simulado
     //Serial.print("pH Simulado: ");
     //Serial.print(ph_valor);
     //Serial.print(" (Tempo: ");
     //Serial.print((tempoAtual - inicio_simulacao) / 1000);
     //Serial.println(" segundos)");
     
     // Lê sensor de umidade (DHT22)
     float umidade_ar = dht.readHumidity();
     
     // Simula sensor de umidade do solo
     if (!isnan(umidade_ar)) {
       umidade_solo = umidade_ar;
     } else {
       // Se o DHT22 não der leitura, usar valores simulados que alternam
       umidade_solo = simularUmidadeTeste();
     }
     
     // Atualiza estado de irrigação baseado nas leituras
     atualizarIrrigacao();
     
     // Exibe os dados no formato CSV para captura fácil via Serial Monitor
     // Formato: timestamp,fosforo,potassio,ph,umidade,irrigacao
     Serial.print(tempoAtual);
     Serial.print(",");
     Serial.print(fosforo_presente ? "1" : "0");
     Serial.print(",");
     Serial.print(potassio_presente ? "1" : "0");
     Serial.print(",");
     Serial.print(ph_valor);
     Serial.print(",");
     Serial.print(umidade_solo);
     Serial.print(",");
     Serial.println(irrigacaoAtiva ? "1" : "0");
     
     // Log detalhado para debug
     //Serial.print("Fósforo: ");
     //Serial.print(fosforo_presente ? "Presente" : "Ausente");
     //Serial.print(" | Potássio: ");
     //Serial.print(potassio_presente ? "Presente" : "Ausente");
     //Serial.print(" | pH: ");
     //Serial.print(ph_valor);
     //Serial.print(" | Umidade: ");
     //Serial.print(umidade_solo);
     //Serial.print("% | Irrigação: ");
     //Serial.println(irrigacaoAtiva ? "ATIVA" : "Desativada");
     
     //Serial.println("================================================");
   }
   
   // Pequena pausa para estabilizar o loop
   delay(100);
 }