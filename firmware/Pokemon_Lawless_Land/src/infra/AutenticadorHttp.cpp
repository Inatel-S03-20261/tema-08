#include "infra/AutenticadorHttp.h"

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>

#include <cstdio>
#include <cstring>

namespace {
// Extrai o valor string de "chave":"valor" de um JSON simples (sem ArduinoJson).
void extrairCampoTexto(const char* json, const char* chave, char* dest,
                       size_t n) {
  dest[0] = '\0';
  char alvo[40];
  snprintf(alvo, sizeof(alvo), "\"%s\"", chave);
  const char* p = strstr(json, alvo);
  if (p == nullptr) return;
  p = strchr(p, ':');
  if (p == nullptr) return;
  ++p;
  while (*p == ' ' || *p == '"') ++p;  // pula espacos e a aspa de abertura
  size_t i = 0;
  while (*p != '\0' && *p != '"' && i + 1 < n) dest[i++] = *p++;
  dest[i] = '\0';
}
}  // namespace

AutenticadorHttp::AutenticadorHttp(const char* baseUrl) : baseUrl_(baseUrl) {}

ResultadoLogin AutenticadorHttp::autenticar(const char* usuario,
                                            const char* senha) {
  ResultadoLogin r;
  if (WiFi.status() != WL_CONNECTED) return r;  // offline -> falha (mock segue)

  char url[160];
  snprintf(url, sizeof(url), "%s/auth/login", baseUrl_);

  char body[192];
  snprintf(body, sizeof(body),
           "{\"usernameOrEmail\":\"%s\",\"password\":\"%s\"}", usuario, senha);

  HTTPClient http;
  http.setConnectTimeout(4000);
  http.setTimeout(4000);
  if (!http.begin(url)) return r;
  http.addHeader("Content-Type", "application/json");

  int status = http.POST(reinterpret_cast<uint8_t*>(body), strlen(body));
  if (status == 200) {
    r.sucesso = true;
    String resp = http.getString();
    extrairCampoTexto(resp.c_str(), "token", r.token, sizeof(r.token));
  }
  http.end();
  return r;
}
