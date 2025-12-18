## Plán krok za krokem, jak kód rozšířit.
Vyřešit 4 klíčové oblasti: Platformní nezávislost, Čistotu (Cleanup), Ne-blokování GUI a Bezpečnost.

### Fáze 1: Skutečná multiplatformnost (Linux & Windows)
Váš kód sice má `#ifdef`, ale `HOME` na Windows nefunguje spolehlivě (tam je to `%USERPROFILE%` nebo `%APPDATA%`). Navíc vytváříte `archiv.zip` v aktuálním pracovním adresáři, což může být na Windows v Program Files zakázané (potřebujete admin práva).

**Co změnit:**
- Logy a dočasný ZIP ukládat do systémových temp adresářů.

- Správně detekovat domovskou složku na Windows.

**Úprava kódu:**

```C++
fs::path get_temp_path() {
    return fs::temp_directory_path() / "diagnostics_upload.zip";
}

fs::path get_home_path() {
#ifdef _WIN32
    const char* user_profile = std::getenv("USERPROFILE");
    return user_profile ? fs::path(user_profile) : fs::path("C:\\");
#else
    const char* home = std::getenv("HOME");
    return home ? fs::path(home) : fs::path("/tmp");
#endif
}

// V main():
fs::path outputZipPath = get_temp_path(); // Místo "archiv.zip"
std::string outputZip = outputZipPath.string();
```

### Fáze 2: RAII a Úklid (Cleanup)
Váš kód po sobě nechává soubor `archiv.zip`. Pokud to aplikace udělá 100x, uživatel bude mít disk plný zipů. Navíc, pokud program spadne uprostřed (mezi zipováním a odesláním), soubor tam zůstane.

**Řešení:** Použijte princip RAII nebo try/catch/finally logiku, aby se soubor smazal vždy, ať už se upload povede nebo ne.

**Úprava kódu:**

```C++
// Jednoduchý helper pro smazání souboru při zničení objektu
struct FileDeleter {
    fs::path p;
    ~FileDeleter() { 
        if (fs::exists(p)) {
            fs::remove(p); 
            std::cout << "Cleanup: Smazan docasny soubor " << p << std::endl;
        }
    }
};

int main() {
    // ... definice cest ...
    
    {
        FileDeleter cleaner{outputZipPath}; // Jakmile tato proměnná zmizí (konec main), soubor se smaže
        
        if (!zipDir(sourceDir, outputZip)) return -1;
        
        // ... CURL logika ...
        // I když CURL selže nebo hodí chybu, destruktor FileDeleteru uklidí.
    }
    return 0;
}
```

### Fáze 3: Ne-blokující odesílání (Pro GUI)
To je kritické. Pokud tento kód dáte do tlačítka v GUI (Qt, ImGui, MFC...), a internet bude pomalý, celá aplikace zamrzne a bude vypadat, že spadla ("Not Responding").

**Plán:** Obalit celou logiku do funkce a spouštět ji v jiném vlákně (std::thread nebo std::async).

**Koncept:**

```C++
#include <future>
#include <thread>

// Přesuňte logiku z main do funkce uploadLogs()
bool uploadLogs() {
    // ... zipování ...
    // ... curl ...
    // return true/false;
}

int main() {
    std::cout << "GUI bezi..." << std::endl;

    // Spustit asynchronně
    auto future = std::async(std::launch::async, uploadLogs);

    // V reálné appce tady GUI běží dál a jen občas kontroluje, zda je future.wait_for(0) hotové
    // Pro demo počkáme:
    bool result = future.get(); 
    
    if (result) std::cout << "Hotovo, sefe!" << std::endl;
}
```

### Fáze 4: Metadata a Identifikace (To co chce backend)
Posíláte soubor, ale backend neví, kdo ho posílá a jakou verzi aplikace má. To je pro debugování k ničemu.

**Rozšíření CURL hlaviček:** Přidejte do HTTP requestu vlastní hlavičky.

```C++
struct curl_slist *headerlist = NULL;
headerlist = curl_slist_append(headerlist, "Expect:"); 
headerlist = curl_slist_append(headerlist, "X-App-Version: 1.0.4-beta");
headerlist = curl_slist_append(headerlist, "X-OS: Windows 11");
headerlist = curl_slist_append(headerlist, "X-User-ID: 12345"); // Nebo vygenerované UUID
curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
```

### Fáze 5: Bezpečnost (HTTPS)
Protože používáte libcurl, je přechod na bezpečný přenos triviální, ale musíte to nastavit.

Změňte URL na `https://...`

Pro lokální testování (self-signed certifikáty) musíte vypnout verifikaci, ale do produkce ji musíte zapnout.

```C++
curl_easy_setopt(curl, CURLOPT_URL, "https://muj-server.com/upload");

// PRODUKCE: Nechat default (ověřuje certifikát)
// VÝVOJ (localhost):
curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); 
curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
```

**Checklist:**

✅ Cross-Platform: "Funguje to spolehlivě na Windows i Linuxu, správně to detekuje cesty k logům."

✅ Zero-Footprint: "Aplikace po sobě uklízí. Dočasné archivy se mažou hned po odeslání."

✅ Non-Blocking: "Odesílání běží na pozadí, takže to nezasekne uživateli rozhraní aplikace."

✅ Identifikace: "Server automaticky pozná verzi aplikace a OS, takže budeme hned vědět, kde se stala chyba."

✅ Bezpečnost: "Je to připraveno na HTTPS přenos."

**Další krok:** Chtěl byste pomoci s tím std::async wrapperem nebo s úpravou toho Flask serveru, aby uměl přijímat ty metadata v hlavičkách?