//VERSÃO 1.0 concatenação
//Importação de Bibliotecas
#include <WiFi.h>  //Importa Biblioteca do módulo WiFi
#include <PubSubClient.h> //Importa Biblioteca de publish e subscribe

//Definição de Tópicos e ID do equipamento
#define TOPICO_SUBSCRIBE "WRITEME"      //Tópico MQTT de leitura
#define TOPICO_PUBLISH   "README_2"    //Tópico MQTT envio de informações
#define ID_MQTT          "NODEMCU1.0_8266"  //ID MQTT do componente

//Variáveis para dados de leitura e armazenamento
int   quant;
int   j = 0;
float leitura;
float tempo;
float tempod = 0;
float grava = 0;
float gravac;

//Variáveis de conexão WiFi
//const char* SSID     = "NET_2GB9DACA"; //Nome da rede wifi a ser conectado
//const char* PASSWORD = "E2B9DACA";     //Senha da rede WiFi a ser conectado
const char* SSID     = "OFFICE_GAMES"; //Nome da rede wifi a ser conectado
const char* PASSWORD = "10392090";     //Senha da rede WiFi a ser conectado

//Variáveis de conexção MQTT
//const char* BROKER_MQTT = "192.168.0.31"; //IP do broker para conexão 
const char* BROKER_MQTT = "test.mosquitto.org"; //IP do broker para conexão 
int         BROKER_PORT = 1883;           //Porta do broker a ser utilizada

//Variáveis e Objetos Globais
WiFiClient espClient;        //Cria o objeto espClient
PubSubClient MQTT(espClient); // Instancia o Cliente MQTT passando o objeto espClient

//Prototypes
void initWiFi();
void initMQTT();
void reconectWiFi();
void VerificaConexoesWiFIEMQTT (void);

//Implementação das funções
void setup() 
{
    Serial.begin (115200); //Inicializa o serial
    initWiFi();
    initMQTT();
    quant = 10;
    tempo = 2000;
    gravac = tempo/quant;
}

void initWiFi() //Função: inicializa e conecta-se na rede WI-FI desejada 
{
    delay(10);
    Serial.println("------Conexao WI-FI------");
    Serial.print("Conectando-se na rede: ");
    Serial.println(SSID);
    Serial.println("Aguarde");
    reconectWiFi();
}

void initMQTT() //Função: inicializa parâmetros de conexão MQTT(endereço do broker, porta e seta função de callback)
{
    MQTT.setServer(BROKER_MQTT, BROKER_PORT);   //informa qual broker e porta deve ser conectado
}

void reconnectMQTT() //Função: reconecta-se ao broker MQTT (caso ainda não esteja conectado ou em caso de a conexão cair)
{                    //        em caso de sucesso na conexão ou reconexão, o subscribe dos tópicos é refeito.
    while (!MQTT.connected()) 
    {
        Serial.print("* Tentando se conectar ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        if (MQTT.connect(ID_MQTT)) 
        {
            Serial.println("Conectado com sucesso ao broker MQTT!");
            MQTT.subscribe(TOPICO_SUBSCRIBE); 
        } 
        else
        {
            Serial.println("Falha ao reconectar no broker.");
            Serial.println("Havera nova tentatica de conexao em 2s");
            delay(2000);
        }
    }
}
  
void reconectWiFi() //Função: reconecta-se ao WiFi
{
    //se já está conectado a rede WI-FI, nada é feito. 
    //Caso contrário, são efetuadas tentativas de conexão
    if (WiFi.status() == WL_CONNECTED)
        return;
         
    WiFi.begin(SSID, PASSWORD); // Conecta na rede WI-FI
     
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(100);
        Serial.print(".");
    }
   
    Serial.println();
    Serial.print("Conectado com sucesso na rede ");
    Serial.print(SSID);
    Serial.println("IP obtido: ");
    Serial.println(WiFi.localIP());
}
 
//Função: verifica o estado das conexões WiFI e ao broker MQTT. 
//        Em caso de desconexão (qualquer uma das duas), a conexão é refeita.
void VerificaConexoesWiFIEMQTT(void)
{
    if (!MQTT.connected())
        reconnectMQTT(); //se não há conexão com o Broker, a conexão é refeita
    reconectWiFi(); //se não há conexão com o WiFI, a conexão é refeita
}

void loop()
{
    char dados [800];
    char dadosAux [800];
            
    if (millis() - grava >= gravac)
    {
        //Serial.println("Vetor dados zerado");
        leitura = analogRead(A0);     // aqui vc vai estar gravando as leituras do pino analogico
        dtostrf(leitura,6,2,dadosAux); // aqui há a conversão de float pra char[] (como se fosse "String")

        strcat(dados,dadosAux); // aqui concatena o conteudo do auxiliar com o dados efetivamente
        strcat(dados, " ");     /* colocando separação entre os valores
        o auxiliar recebe sempre 1 valor, então ele vai sempre ter 1 valor, pois está sobrepondo,
        por isso preciso de 2 vetores e de concatenação*/
        
        grava = millis();
    }
    // aqui, todas as leituras estão em dados, e quando der o tempo para publicar, é que vamos colocar
    // a palavra "Pacote: " no início do vetor 

    if (millis() - tempod >= tempo)
    {
        //logica para colocar "Pacote: " no começo do vetor
        dadosAux[0] = '\0';           // zerando dadosAux
        strcat(dadosAux, "Pacote: "); //dadosAux recebe pacote
        strcat(dadosAux, dados);

        MQTT.publish(TOPICO_PUBLISH, dadosAux);        //Linha de efetivo envio do dado
        //Serial.println(dadosAux);
        tempod = millis();
        dados[0] = '\0'; //zerando o vetor dados
        dadosAux[0] = '\0';
    }
      
    VerificaConexoesWiFIEMQTT(); //garante funcionamento das conexões WiFi e ao broker MQTT
    MQTT.loop();                 //keep-alive da comunicação com broker MQTT
}
