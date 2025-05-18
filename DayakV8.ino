#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <vector>
#include <DNSServer.h>
#include <ESPmDNS.h>

// AP Configuration
const char* AP_SSID = "DayakTranslator";  // Nama jaringan WiFi yang akan muncul
const char* AP_PASSWORD = "12345678";      // Password untuk koneksi WiFi
const byte DNS_PORT = 53;                  // Port DNS standar
const char* DNS_DOMAIN = "dayak.local";    // Domain lokal untuk akses web
IPAddress AP_IP(192, 168, 4, 1);          // IP address untuk akses web (192.168.4.1)
IPAddress AP_SUBNET(255, 255, 255, 0);     // Subnet mask

// Initialize servers
WebServer server(80);
DNSServer dnsServer;
StaticJsonDocument<16384> dict;

// Connection tracking
struct ClientInfo {
    IPAddress ip;
    unsigned long lastActivity;
};
std::vector<ClientInfo> activeClients;
const unsigned long CLIENT_TIMEOUT = 300000; // 5 minutes in milliseconds

// KAMUS DAYAK KENYAH
const char dictionary_json[] PROGMEM = R"rawliteral(
 {
    "apa": "inu",
    "kemana": "kenpi",
    "dimana": "kenpi",
    "siapa": "ahe",
    "kapan": "mi'an",
    "bagaimana": "kumpin",
    "putih": "putek",
    "hitam": "saleng",
    "merah": "bala",
    "1": "ca'",
    "2": "dua",
    "3": "telu",
    "4": "pat",
    "5": "lema",
    "6": "nem",
    "7": "tujuk",
    "8": "aya",
    "9": "pe'en",
    "10": "puluk",
    "11": "ca' elas",
    "12": "dua elas",
    "13": "telu elas",
    "14": "pat elas",
    "15": "lema elas",
    "16": "nem elas",
    "17": "tujuk elas",
    "18": "aya elas",
    "19": "pe'en elas",
    "20": "dua puluk",
    "30": "telu puluk",
    "40": "pat puluk",
    "50": "lema puluk",
    "60": "nem puluk",
    "70": "tujuk puluk",
    "80": "aya puluk",
    "90": "pe'en puluk",
    "100": "ca' ato",
    "1000": "ca' ibu",
    "1000000": "ca' juta",
    "satu": "ca'",
    "dua": "dua",
    "tiga": "telu",
    "empat": "pat",
    "lima": "lema",
    "enam": "nem",
    "tujuh": "tujuk",
    "delapan": "aya",
    "sembilan": "pe'en",
    "sepuluh": "puluk",
    "sebelas": "ca' elas",
    "dua belas": "dua elas",
    "tiga belas": "telu elas",
    "empat belas": "pat elas",
    "lima belas": "lema elas",
    "enam belas": "nem elas",
    "tujuh belas": "tujuk elas",
    "delapan belas": "aya elas",
    "sembilan belas": "pe'en elas",
    "dua puluh": "dua puluk",
    "tiga puluh": "telu puluk",
    "empat puluh": "pat puluk",
    "lima puluh": "lema puluk",
    "enam puluh": "nem puluk",
    "tujuh puluh": "tujuk puluk",
    "delapan puluh": "aya puluk",
    "sembilan puluh": "pe'en puluk",
    "seratus": "ca' ato",
    "seribu": "ca' ibu",
    "sejuta": "ca' juta",
    "ratus": "ato",
    "ribu": "ibu",
    "belas": "elas",
    "nya": "la",
    "aku": "ake",
    "ada": "aun",
    "anjing": "udut",
    "ayam": "yap",
    "babi": "babui",
    "banteng": "kalasiau",
    "bebek": "bidik",
    "belalang": "kade",
    "beruang": "buang",
    "biawak": "kabuk",
    "buaya": "baya",
    "burung": "sui",
    "cicak": "aru",
    "gajah": "kesun",
    "harimau": "lenjau",
    "ikan": "atuk",
    "lele": "kati",
    "patin": "selareng",
    "kambing": "kadeng",
    "kancil": "pelanuk",
    "kecoa": "lepa",
    "kepiting": "yu",
    "kijang": "payak",
    "kodok": "sui",
    "kucing": "sing",
    "kupu-kupu": "bamper",
    "lalat": "langau",
    "cacing": "lati",
    "macan": "kule",
    "monyet": "kuyata",
    "nyamuk": "jamok",
    "rusa": "uca",
    "siput": "sik/ulung",
    "tikus": "belabau",
    "ular": "tuduk",
    "bagus": "tia",
    "baru": "mading",
    "besar": "bio",
    "cepat": "sangit",
    "kelakuan": "puyan",
    "kosong": "mpi un",
    "malas": "malat",
    "nyata": "lan",
    "pelit": "ma'em",
    "ringan": "nyian",
    "sehat": "agat",
    "pagi": "tading nempam",
    "siang": "naktau",
    "malam": "nataup",
    "sibuk": "lamak",
    "berbagai": "kado",
    "atau": "ataw",
    "dan": "ngan",
    "dari": "cen",
    "berasal": "cen",
    "diantara": "dalau",
    "juga": "pe",
    "karena": "uban",
    "ke": "koq",
    "mending": "lepa ini",
    "saat": "sa taup",
    "sebelum": "ken'a",
    "tapi": "untepu'o",
    "tetapi": "untepu'o",
    "setelah": "lepa kadi",
    "supaya": "apan",
    "agar": "apan",
    "untuk": "un",
    "yang": "yak",
    "adalah": "ja ne",
    "ialah": "ialah",
    "kalau": "awa'",
    "badan": "usa",
    "baju": "sapai",
    "buah": "buaq",
    "keledang": "ta'ep",
    "cempedak": "nakan",
    "lay": "pakit",
    "rambutan": "sangit",
    "desa": "lepo",
    "gula": "gula'k",
    "jendela": "sekan",
    "kamar": "tilung lundo",
    "kursi": "adan",
    "lemari": "buan",
    "lontong": "kanen lema'k",
    "makanan": "penguman",
    "memakan": "uman",
    "dimakan": "uman",
    "meja": "mejak",
    "minuman": "nisep ia",
    "minyak": "lenya",
    "mobil": "oto",
    "goreng": "saga",
    "pasar": "pasen",
    "piring": "kiyat",
    "pondok": "lepau",
    "rumah": "amin",
    "sungai": "alo",
    "tas": "bek",
    "telur": "telo",
    "kandang": "liwang",
    "sudah": "lepa",
    "sampai": "pa",
    "balik": "liwer",
    "bangun": "taga",
    "belajar": "pekalai",
    "berangkat": "tai te",
    "bermain": "piat",
    "berpikir": "pekimet",
    "bikin": "uyan",
    "buat": "uyan",
    "bilang": "kun",
    "sampaikan": "kun",
    "bohong": "pala",
    "bolos": "tai kelap",
    "jalan": "masat",
    "jalan-jalan": "masat-masat",
    "jangan": "ain",
    "keluar": "kawang",
    "bekerja": "gayeng",
    "kerja": "gayeng",
    "kerjakan": "gayeng",
    "ketahui": "tisen",
    "tau": "tisen",
    "makan": "uman",
    "mandi": "ndu'",
    "masak": "pesak",
    "masuk": "ke dalem",
    "melihat": "ngeneng",
    "melihatnya": "ngeneng ia",
    "membeli": "meli",
    "mendengar": "ngeninga",
    "menyanyi": "ngendau",
    "minum": "nisep",
    "nyaring": "bia",
    "singgah": "ngena'k",
    "tinggal": "ngena'k",
    "disini": "ki",
    "terima": "tiga",
    "kasih": "tawai",
    "tidur": "lundog",
    "pindah": "buaw",
    "berteduh": "ngedingan",
    "petubo": "pelihara",
    "adegan": "kaa",
    "banyak": "kado",
    "sekali": "ale",
    "boleh": "kong",
    "dulu": "ading",
    "hari": "tau",
    "ingatan": "kimet",
    "kemarin": "nya'ep rei",
    "memang": "adang",
    "suka": "uba",
    "cinta": "uba",
    "sayang": "uba",
    "melewati": "lapa",
    "sama": "kuwa",
    "nama": "ngadan",
    "namaku": "ngadan keq",
    "ku": "keq",
    "oke": "aha",
    "iya": "aha",
    "pulang": "ula",
    "sangat": "ali'",
    "semua": "mung",
    "seperti": "kuaq",
    "suaranya": "isu'a",
    "tentu": "ina",
    "saja": "na",
    "tidak": "nda",
    "gayeng": "tugas",
    "sekolah": "sekula",
    "kabar": "denga",
    "berita": "denga",
    "hal": "denga",
    "kejadian": "denga",
    "salah": "bep",
    "datang": "nay",
    "tersisa": "naung",
    "bapak": "amai",
    "bersama": "ilu mung",
    "hantu": "bali",
    "ibu": "uweq",
    "kalian": "ikem",
    "kami": "ame",
    "kamu": "ikoq",
    "kau": "ikoq",
    "kita": "ilu",
    "saya": "ake",
    "teman": "tuyang",
    "mereka": "ida",
    "masing": "tengen",
    "masing-masing": "tengen-tengen",
    "senin": "keca",
    "selasa": "kedua",
    "rabu": "ketelu",
    "kamis": "tepat",
    "jumat": "kelema",
    "sabtu": "kenem",
    "minggu": "minggu",
    "adik": "ari'",
    "angin": "bayu",
    "air": "sungai",
    "rumahmu": "lamin engku",
    "ambil": "alak",
    "baik": "lemeli",
    "berenang": "nyatung",
    "bocor": "tudok",
    "berak": "lepin",
    "berdiri": "ngrejeng",
    "bicara": "petirak",
    "bintang": "betuen",
    "besok": "nembam",
    "banjir": "layap",
    "berapa": "kudak",
    "celana": "seluen",
    "cabe": "lia",
    "duduk": "adung",
    "dingin": "nyeng",
    "gelap": "mendem",
    "gigi": "jipen",
    "gemuk": "lembo",
    "gendong": "bak",
    "gunung": "mudung",
    "gundul": "ulok",
    "garam": "usen",
    "hidup": "mudip",
    "hidung": "ndung",
    "hutan": "a'ut",
    "haus": "moang laset",
    "jernih": "litut",
    "mamak": "tinam",
    "jatuh": "labok",
    "jauh": "cok",
    "jelek": "ja'at",
    "kencing": "nyengit",
    "kenyang": "beso",
    "kepala": "ulu",
    "keatas": "kempou",
    "naik": "kempou",
    "kembali": "ulek",
    "kemarau": "tagak",
    "kakek": "pui",
    "nenek": "pui",
    "kakak": "seken",
    "kaki": "taket",
    "kurus": "mae",
    "kecil": "ang",
    "lihat": "maat",
    "lapar": "lau",
    "laki": "laki",
    "lidah": "uma",
    "alkohol": "jakan",
    "mabok": "meduk",
    "madu": "layuk",
    "orang": "lakeng",
    "mencari": "mita",
    "malu": "sa'e",
    "menyelam": "lemeset",
    "memanjat": "lemaket",
    "menanam": "mula",
    "menari": "kanjet",
    "mau": "obak",
    "mata": "mata",
    "mimpi": "ngenupi",
    "mencuri": "mengelau",
    "menangis": "menange",
    "mana": "mpi",
    "mati": "matae",
    "menyeberang": "pelawat",
    "peluk": "nyekapu",
    "panen": "maju",
    "perut": "batek",
    "pahit": "pait",
    "pintu": "pamen",
    "pakaian": "sapai",
    "perahu": "alut",
    "parang": "mandau",
    "panjang": "dadok",
    "pendek": "bu'et",
    "pedas": "sanit",
    "pergi": "tei",
    "pandai": "encam",
    "pohon": "puun",
    "rindu": "menok menawai",
    "sayur": "lekey",
    "suami mu": "laki koq",
    "satu bulan": "ca bulan",
    "setuju": "aha",
    "selimut": "kelumah",
    "benar": "sio",
    "di": "ka",
    "sini": "ini",
    "sana": "ina",
    "situ": "ina",
    "berlari": "kasa",
    "berbaring": "meken",
    "bersandar": "nyendei",
    "berkebun": "uyam pula",
    "nasi": "kenan",
    "pedagang": "toke",
    "nangis": "nange",
    "main": "piat",
    "ya": "aha",
    "antara": "aang",
    "hilir": "aba",
    "peraturan": "adet",
    "kenapa": "adi",
    "mengapa": "adi",
    "kesatu": "ading",
    "dahulu": "ading",
    "terdahulu": "ading",
    "lalang": "ai'",
    "pasir": "ait",
    "tuntun": "ajak",
    "baris": "ajat",
    "keberanian": "akang",
    "memberanikan": "ngakang",
    "berani": "makang",
    "ide": "aken",
    "pikiran": "aken",
    "kebijaksanaan": "aken",
    "pinggang": "aking",
    "rakit": "akit",
    "kapal": "akit",
    "meraup": "makup",
    "cupu": "a'i'",
    "jantan": "a'ung",
    "belukar": "ma'ut",
    "semak": "ma'ut",
    "bersemak": "ma'ut",
    "berumput": "ma'ut",
    "rumput": "ma'ut",
    "dapat": "ala'",
    "mendapatkan": "ala'",
    "bisa": "ala'",
    "ranting": "alan",
    "setengah": "alang",
    "kebiasaan": "alay",
    "lazim": "alay",
    "jalur": "ale",
    "banding": "aliu",
    "arus": "alo",
    "mengukur": "alut",
    "simpan": "mambin",
    "menyimpan": "mambin",
    "memegang": "mambin",
    "pegang": "mambin",
    "memelihara": "mambim",
    "tersimpan": "pambin",
    "rantai": "ambit",
    "bodoh": "ameng",
    "bisu": "ameng",
    "keluarga": "amit",
    "mungkin": "amu'",
    "kekanakkanakan": "pekanak",
    "kembar": "pit",
    "saudara": "aung",
    "saudari": "aung",
    "menatap": "aneng",
    "tatap": "aneng",
    "kulit": "anit",
    "punya": "anun",
    "milik": "anun",
    "rintangan": "apat",
    "aral": "apat",
    "halangan": "apat",
    "terlintang": "papat",
    "tertahan": "papat",
    "terhalang": "papat",
    "menara": "apaw",
    "penara": "apaw",
    "bukit": "apaw",
    "tali": "apay",
    "cacat": "apet",
    "kapur": "apo",
    "tertarik": "arep",
    "berminat": "arep",
    "keinginan": "arep",
    "cenderung": "arep",
    "lahap": "asam",
    "rakus": "asam",
    "likut": "mundur",
    "asal": "asen",
    "jurang": "asep",
    "lembah": "asep",
    "beratus": "ato",
    "meletakkan": "maung",
    "menempatkan": "maung",
    "menaruh": "maung",
    "andai": "awa'",
    "basi": "basi",
    "basa'": "basah",
    "batu": "batu",
    "bayar": "bayan",
    "teriak": "bika'",
    "kuning": "bila'",
    "pesawat": "bilun",
    "racun": "bisa'",
    "sisir": "bit",
    "cemas": "bisaw",
    "buku": "buk",
    "bersih": "bui'",
    "bola": "bula",
    "aneh": "bulen",
    "gila": "buling",
    "bom": "bum",
    "bermusuhan": "pebunu'",
    "selamat": "bunut",
    "memar": "buok",
    "marah": "busi'",
    "botol": "butun",
    "lain": "cena'an",
    "darah": "daa'",
    "janji": "daji'",
    "tangkap": "dakep",
    "ketika": "dalaw",
    "daerah": "dalé",
    "sesat": "daru",
    "alat": "de'buat",
    "terang": "dema",
    "hemat": "demun",
    "lambat": "dena'",
    "kuat": "denggep",
    "tahu": "tisen",
    "lampu": "titiw",
    "bengkak": "tukuk",
    "lupa": "turan",
    "susu": "tusu",
    "muntah": "tuta'",
    "dermaga": "tuun",
    "hujan": "ujan",
    "tangan": "uju' ",
    "langsung": "umba",
    "dada": "usuk",
    "otak": "utek",
    "membuat": "uyan"
  }
)rawliteral";

// 3. KODE HTML/CSS/JS
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="id">
<head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=0"/>
    <title>Translator & Engine Dayak Kenyah - Offline</title>
    <style>
        :root {
            --bg: #f0f8ff;
            --card: #ffffff;
            --text: #1c3d5a;
            --text-secondary: #5a768e;
            --accent: #007bff;
            --accent-hover: #0056b3;
            --input: #e6f2ff;
            --border-color: #cce5ff;
            --radius: 12px;
            --shadow-color: rgba(0, 123, 255, 0.1);
            --canvas-particle-color: rgba(0, 123, 255, 0.7);
            --canvas-line-color: rgba(0, 123, 255, 0.2);
            --toggle-bg: #e6f2ff;
            --toggle-icon: #007bff;
        }

        [data-theme="dark"] {
            --bg: #1a1a2e;
            --card: #2a2a3e;
            --text: #e0e0ff;
            --text-secondary: #a0a0c0;
            --accent: #60a5fa;
            --accent-hover: #3b82f6;
            --input: #2f2f4f;
            --border-color: #3a3a5e;
            --shadow-color: rgba(96, 165, 250, 0.1);
            --canvas-particle-color: rgba(96, 165, 250, 0.7);
            --canvas-line-color: rgba(96, 165, 250, 0.2);
            --toggle-bg: #2f2f4f;
            --toggle-icon: #60a5fa;
        }

        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
            transition: background-color 0.3s ease, color 0.3s ease, border-color 0.3s ease, opacity 0.3s ease;
            -webkit-tap-highlight-color: transparent;
        }

        html, body {
            position: relative;
            width: 100%;
            height: 100%;
            margin: 0;
            padding: 0;
            overflow-x: hidden;
            overflow-y: auto;
            -webkit-overflow-scrolling: touch;
        }

        body {
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
            background-color: var(--bg);
            color: var(--text);
            line-height: 1.6;
            touch-action: pan-y;
            overscroll-behavior-y: contain;
        }

        #interactive-preloader {
            position: fixed;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background-color: var(--bg);
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            z-index: 9999;
            transition: opacity 0.8s ease 0.2s, visibility 0.8s ease 0.2s;
            overflow: hidden;
            touch-action: none;
        }

        #preloader-canvas-container {
            position: absolute;
            inset: 0;
            z-index: 1;
        }

        #preloader-canvas {
            width: 100%;
            height: 100%;
            display: block;
        }

        .preloader-content {
            text-align: center;
            z-index: 2;
            padding: 2rem;
            border-radius: var(--radius);
            opacity: 0;
            transform: translateY(20px);
            animation: fadeInPreloaderContent 1s ease 0.5s forwards;
            max-width: 90%;
        }

        @keyframes fadeInPreloaderContent {
            to {
                opacity: 1;
                transform: translateY(0);
            }
        }

        .preloader-title {
            font-size: 1.8rem;
            font-weight: 700;
            color: var(--accent);
            margin-bottom: 1rem;
        }

        .preloader-message {
            font-size: 1.1rem;
            color: var(--text-secondary);
            max-width: 500px;
            margin: 0 auto;
            line-height: 1.5;
        }

        .loading-dots {
            display: flex;
            justify-content: center;
            margin-top: 2rem;
        }

        .loading-dots span {
            width: 10px;
            height: 10px;
            margin: 0 5px;
            background-color: var(--accent);
            border-radius: 50%;
            opacity: 0.3;
            animation: pulseDots 1.4s infinite ease-in-out both;
        }

        .loading-dots span:nth-child(1) { animation-delay: -0.32s; }
        .loading-dots span:nth-child(2) { animation-delay: -0.16s; }
        .loading-dots span:nth-child(3) { animation-delay: 0; }

        @keyframes pulseDots {
            0%, 80%, 100% { transform: scale(0.8); opacity: 0.3; }
            40% { transform: scale(1.0); opacity: 1; }
        }

        #interactive-preloader.hidden {
            opacity: 0;
            visibility: hidden;
            pointer-events: none;
        }

        .theme-toggle {
            position: fixed;
            top: 15px;
            right: 15px;
            width: 45px;
            height: 45px;
            background-color: var(--toggle-bg);
            color: var(--toggle-icon);
            border-radius: 50%;
            display: flex;
            align-items: center;
            justify-content: center;
            cursor: pointer;
            z-index: 10000;
            box-shadow: 0 2px 5px rgba(0,0,0,0.1);
            font-size: 1.4rem;
            border: 1px solid var(--border-color);
        }

        .theme-toggle:hover {
            transform: scale(1.1);
            box-shadow: 0 4px 10px rgba(0,0,0,0.15);
        }

        .container {
            position: relative;
            max-width: 800px;
            margin: 2rem auto;
            padding: 1.5rem;
            opacity: 0;
            transform: translateY(20px);
            transition: opacity 0.6s ease 0.2s, transform 0.6s ease 0.2s;
            min-height: calc(100vh - 4rem);
        }

        body.loaded .container {
            opacity: 1;
            transform: translateY(0);
        }

        header {
            text-align: center;
            margin-bottom: 2.5rem;
        }

        .text-logo {
            font-size: 2rem;
            font-weight: bold;
            color: var(--accent);
            margin-bottom: 0.5rem;
            display: inline-block;
            padding: 0.5rem 1rem;
            border: 2px solid var(--accent);
            border-radius: var(--radius);
            background-color: var(--card);
        }

        h1 {
            font-weight: 600;
            margin-bottom: 0.5rem;
            font-size: 1.9rem;
            color: var(--text);
        }

        .description {
            font-size: 1rem;
            color: var(--text-secondary);
        }

        .card {
            background: var(--card);
            border-radius: var(--radius);
            padding: 1.75rem;
            box-shadow: 0 5px 15px var(--shadow-color);
            margin-bottom: 2rem;
            border: 1px solid var(--border-color);
        }

        h2 {
            font-size: 1.4rem;
            font-weight: 600;
            margin-bottom: 1.5rem;
            text-align: center;
            color: var(--accent);
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 0.5rem;
        }

        textarea, input[type="text"], select {
            width: 100%;
            padding: 0.9rem 1rem;
            border-radius: calc(var(--radius) / 1.5);
            border: 1px solid var(--border-color);
            background: var(--input);
            color: var(--text);
            margin-top: 0.75rem;
            font-family: inherit;
            font-size: 1rem;
            resize: vertical;
        }

        textarea:focus, input[type="text"]:focus, select:focus {
            outline: none;
            border-color: var(--accent);
            background-color: var(--card);
            box-shadow: 0 0 0 3px rgba(0, 123, 255, 0.2);
        }

        select {
            appearance: none;
            background-image: url('data:image/svg+xml;charset=US-ASCII,%3Csvg%20xmlns=%22http://www.w3.org/2000/svg%22%20width=%22292.4%22%20height=%22292.4%22%3E%3Cpath%20fill=%22%23007bff%22%20d=%22M287%2069.4a17.6%2017.6%200%200%200-13-5.4H18.4c-5%200-9.3%201.8-12.9%205.4A17.6%2017.6%200%200%200%200%2082.2c0%205%201.8%209.3%205.4%2012.9l128%20127.9c3.6%203.6%207.8%205.4%2012.8%205.4s9.2-1.8%2012.8-5.4L287%2095c3.5-3.5%205.4-7.8%205.4-12.8%200-5-1.9-9.2-5.5-12.8z%22/%3E%3C/svg%3E');
            background-repeat: no-repeat;
            background-position: right 1rem center;
            background-size: 0.8em auto;
            padding-right: 2.5rem;
            cursor: pointer;
        }

        button {
            background: var(--accent);
            color: #ffffff;
            padding: 0.8rem 1.75rem;
            border-radius: calc(var(--radius) / 1.5);
            border: none;
            font-weight: 500;
            font-size: 1rem;
            margin-top: 1rem;
            cursor: pointer;
            display: block;
            width: 100%;
            transition: background-color 0.2s ease, transform 0.1s ease;
        }

        button:hover {
            background: var(--accent-hover);
        }

        button:active {
            transform: scale(0.98);
        }

        .output {
            margin-top: 1.5rem;
            padding: 1rem;
            background: var(--input);
            border-radius: calc(var(--radius) / 1.5);
            min-height: 50px;
            border: 1px dashed var(--border-color);
            color: var(--text-secondary);
            font-style: italic;
            word-wrap: break-word;
        }

        .output.has-content {
            color: var(--text);
            font-style: normal;
            border-style: solid;
            background-color: var(--bg);
        }

        .loader {
            border: 4px solid var(--input);
            border-top: 4px solid var(--accent);
            border-radius: 50%;
            width: 24px;
            height: 24px;
            animation: spin 0.8s linear infinite;
            margin: 1rem auto;
            display: none;
        }

        @keyframes spin {
            0% { transform: rotate(0deg); }
            100% { transform: rotate(360deg); }
        }

        .shake {
            animation: shake 0.4s cubic-bezier(.36,.07,.19,.97) both;
        }

        @keyframes shake {
            10%, 90% { transform: translateX(-1px); }
            20%, 80% { transform: translateX(2px); }
            30%, 50%, 70% { transform: translateX(-3px); }
            40%, 60% { transform: translateX(3px); }
        }

        footer {
            text-align: center;
            font-size: 0.9rem;
            color: var(--text-secondary);
            margin-top: 3rem;
            padding: 1.5rem;
            border-top: 1px solid var(--border-color);
        }

        .instagram-link {
            display: inline-flex;
            align-items: center;
            gap: 8px;
            color: var(--accent);
            text-decoration: none;
            font-weight: 500;
        }

        .instagram-link:hover {
            background-color: var(--input);
        }

        /* Ukuran ikon Instagram seukuran teks */
        .instagram-icon {
            width: 1em;
            height: 1em;
            vertical-align: text-bottom;
            fill: currentColor;
        }

        @media (max-width: 768px) {
            .container { margin: 1rem auto; padding: 1rem; min-height: calc(100vh - 2rem); }
            h1 { font-size: 1.7rem; }
            h2 { font-size: 1.2rem; }
            textarea, input[type="text"], select { padding: 0.8rem; font-size: 0.95rem; }
            button { padding: 0.7rem 1.5rem; font-size: 0.95rem; }
        }

        @media (max-width: 480px) {
            .container { margin: 0.5rem auto; padding: 0.8rem; }
            h1 { font-size: 1.4rem; }
            textarea, input[type="text"], select { padding: 0.7rem; font-size: 0.9rem; }
            button { padding: 0.6rem 1.2rem; font-size: 0.9rem; }
        }
    </style>
</head>
<body ontouchstart="">

    <div class="theme-toggle" id="theme-toggle" onclick="toggleTheme()" title="Ganti Tema">
        <span id="theme-icon"></span>
    </div>

    <div id="interactive-preloader">
        <div id="preloader-canvas-container">
            <canvas id="preloader-canvas"></canvas>
        </div>
        <div class="preloader-content">
            <div class="preloader-title">Dayak Kenyah Translator Pro</div>
            <div class="preloader-message">Dari ide kecil lahirlah dampak besar bagi banyak orang.</div>
            <div class="loading-dots">
                <span></span><span></span><span></span>
            </div>
        </div>
    </div>

    <div class="container">
        <header>
            <div class="text-logo">DKT</div>
            <h1>Dayak Kenyah Translator Pro</h1>
            <p class="description">Translator + MC + Fill-in Engine. Antarmuka Offline.</p>
        </header>

        <section class="card">
            <h2><span aria-hidden="true">🔄</span> Translator</h2>
            <p><strong>Translate dari:</strong> <span id="currentLang">Bahasa Indonesia</span></p>
            <textarea id="inputText" placeholder="Tulis teks di sini..." rows="4"></textarea>
            <button onclick="swapLanguage()">Ganti Bahasa (<span id="swapLangTarget">Dayak Kenyah</span>)</button>
            <div class="loader" id="translatorLoader"></div>
            <div class="output" id="outputText">Hasil terjemahan akan muncul di sini.</div>
        </section>

        <section class="card">
            <h2><span aria-hidden="true">📝</span> Multiple Choice Engine</h2>
            <textarea id="mcQuestion" placeholder="Masukkan pertanyaan..." rows="3"></textarea>
            <input type="text" id="mcOptions" placeholder="Pilihan jawaban dipisahkan koma (cth: A,B,C,D)" />
            <select id="mcLang">
                <option value="id_to_dyk">Soal ID → Jawaban Dayak</option>
                <option value="dyk_to_id">Soal Dayak → Jawaban ID</option>
            </select>
            <button onclick="submitMC()">Proses Soal Pilihan Ganda</button>
            <div class="loader" id="mcLoader"></div>
            <div class="output" id="mcResult">Hasil pemrosesan soal pilihan ganda akan muncul di sini.</div>
        </section>

        <section class="card">
            <h2><span aria-hidden="true">✍️</span> Fill-in Answer Engine</h2>
            <select id="fillQuestionLang">
                <option value="id">Soal Bahasa Indonesia</option>
                <option value="dyk">Soal Bahasa Dayak</option>
            </select>
            <select id="fillOutputLang">
                <option value="dyk">Jawaban Bahasa Dayak</option>
                <option value="id">Jawaban Bahasa Indonesia</option>
            </select>
            <textarea id="fillQuestion" placeholder="Tulis soal isian di sini..." rows="3"></textarea>
            <button onclick="submitFill()">Proses Soal Isian</button>
            <div class="loader" id="fillLoader"></div>
            <div class="output" id="fillResult">Hasil pemrosesan soal isian akan muncul di sini.</div>
        </section>

        <footer>
            <p>© Muhammad Rizky Saputra</p>
            <p>XI TJKT 2, 2025</p>
            <p>SMK Negeri 7 Samarinda</p>
            <p>
                <a href="#" class="instagram-link">
                    <svg class="instagram-icon" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg">
                        <path d="M12 2c2.717 0 3.056.01 4.122.06 1.065.05 1.79.217 2.428.465.66.254 1.216.598 1.772 1.153.509.5.902 1.105 1.153 1.772.247.637.415 1.363.465 2.428.047 1.066.06 1.405.06 4.122 0 2.717-.01 3.056-.06 4.122-.05 1.065-.218 1.79-.465 2.428a4.883 4.883 0 0 1-1.153 1.772c-.5.508-1.105.902-1.772 1.153-.637.247-1.363.415-2.428.465-1.066.047-1.405.06-4.122.06-2.717 0-3.056-.01-4.122-.06-1.065-.05-1.79-.218-2.428-.465a4.89 4.89 0 0 1-1.772-1.153 4.904 4.904 0 0 1-1.153-1.772c-.248-.637-.415-1.363-.465-2.428C2.013 15.056 2 14.717 2 12c0-2.717.01-3.056.06-4.122.05-1.066.217-1.79.465-2.428a4.88 4.88 0 0 1 1.153-1.772A4.897 4.897 0 0 1 5.45 2.525c.638-.248 1.362-.415 2.428-.465C8.944 2.013 9.283 2 12 2zm0 5a5 5 0 1 0 0 10 5 5 0 0 0 0-10zm6.5-.25a1.25 1.25 0 1 0-2.5 0 1.25 1.25 0 0 0 2.5 0zM12 9a3 3 0 1 1 0 6 3 3 0 0 1 0-6z"/>
                    </svg>
                    @kysukamieayam
                </a>
            </p>
        </footer>
    </div>

    <script>
    // Error handling to debug loading issues
    window.onerror = function(message, source, lineno, colno, error) {
        console.error('JavaScript Error:', message, 'at', source, ':', lineno, ':', colno);
        // Create a visible error display for debugging if preloader is still active
        if (document.getElementById('interactive-preloader') && 
            !document.getElementById('interactive-preloader').classList.contains('hidden')) {
            let errorDiv = document.createElement('div');
            errorDiv.style.position = 'fixed';
            errorDiv.style.bottom = '10px';
            errorDiv.style.left = '10px';
            errorDiv.style.right = '10px';
            errorDiv.style.backgroundColor = '#ffeeee';
            errorDiv.style.color = '#cc0000';
            errorDiv.style.padding = '10px';
            errorDiv.style.borderRadius = '5px';
            errorDiv.style.zIndex = '10000';
            errorDiv.style.fontSize = '12px';
            errorDiv.style.maxHeight = '150px';
            errorDiv.style.overflow = 'auto';
            errorDiv.innerHTML = '<strong>Debug Error:</strong> ' + message;
            document.body.appendChild(errorDiv);
            
            // Force hide preloader after 8 seconds total regardless of errors
            setTimeout(() => {
                document.getElementById('interactive-preloader').classList.add('hidden');
                document.body.classList.add('loaded');
            }, 8000);
        }
        return false;
    };
    
    // Check browser capabilities
    const checkBrowser = () => {
        let result = {
            fetch: typeof fetch !== 'undefined',
            promises: typeof Promise !== 'undefined',
            localStorage: (function() {
                try {
                    localStorage.setItem('test', 'test');
                    localStorage.removeItem('test');
                    return true;
                } catch(e) {
                    return false;
                }
            })(),
            json: typeof JSON !== 'undefined'
        };
        
        console.log('Browser capability check:', result);
        return result;
    };
    
    // Run browser check
    const browserCapabilities = checkBrowser();
    
    let currentTranslateLang = 'id';
    let currentTheme = (browserCapabilities.localStorage && localStorage.getItem('theme')) || 'light';

    const htmlEl = document.documentElement;
    const themeIcon = document.getElementById('theme-icon');
    const preloader = document.getElementById('interactive-preloader');

    function applyTheme(theme) {
        htmlEl.setAttribute('data-theme', theme);
        themeIcon.textContent = theme === 'dark' ? '☀️' : '🌙';
        localStorage.setItem('theme', theme);
    }

    document.getElementById('theme-toggle').onclick = () => {
        applyTheme(currentTheme === 'light' ? (currentTheme = 'dark') : (currentTheme = 'light'));
    };

    applyTheme(currentTheme);

    // Canvas particle preloader
    const canvas = document.getElementById('preloader-canvas');
    const ctx = canvas.getContext('2d');
    let particles = [], mouse = { x: null, y: null, radius: 80 };

    function resizeCanvas() {
        canvas.width = window.innerWidth;
        canvas.height = window.innerHeight;
    }

    class Particle {
        constructor(x, y) {
            this.x = x; this.y = y;
            this.size = Math.random() * 4 + 1;
            this.density = Math.random() * 20 + 5;
            this.vx = (Math.random() - 0.5) * 0.5;
            this.vy = (Math.random() - 0.5) * 0.5;
        }
        draw() {
            ctx.fillStyle = getComputedStyle(htmlEl).getPropertyValue('--canvas-particle-color');
            ctx.beginPath();
            ctx.arc(this.x, this.y, this.size, 0, Math.PI * 2);
            ctx.fill();
        }
        update() {
            if (mouse.x !== null && mouse.y !== null) {
                let dx = mouse.x - this.x;
                let dy = mouse.y - this.y;
                let dist = Math.hypot(dx, dy) || 0.1;
                let force = Math.max((mouse.radius - dist) / mouse.radius, 0);
                let dirX = (dx / dist) * force * this.density * 0.1;
                let dirY = (dy / dist) * force * this.density * 0.1;
                if (dist < mouse.radius) {
                    this.x -= dirX; this.y -= dirY;
                }
            }
            this.x += this.vx; this.y += this.vy;
            if (this.x < -this.size) this.x = canvas.width + this.size;
            if (this.x > canvas.width + this.size) this.x = -this.size;
            if (this.y < -this.size) this.y = canvas.height + this.size;
            if (this.y > canvas.height + this.size) this.y = -this.size;
        }
    }

    function initParticles() {
        particles = [];
        let count = Math.min(150, (canvas.width * canvas.height) / 10000);
        for (let i = 0; i < count; i++) {
            particles.push(new Particle(Math.random() * canvas.width, Math.random() * canvas.height));
        }
    }

    function connectParticles() {
        for (let a = 0; a < particles.length; a++) {
            for (let b = a + 1; b < particles.length; b++) {
                let dx = particles[a].x - particles[b].x;
                let dy = particles[a].y - particles[b].y;
                let dist = Math.hypot(dx, dy);
                if (dist < 70) {
                    let opacity = 1 - dist / 70;
                    ctx.strokeStyle = getComputedStyle(htmlEl).getPropertyValue('--canvas-line-color').replace(/[\d\.]+\)$/, (opacity * 0.5) + ')');
                    ctx.lineWidth = 1;
                    ctx.beginPath();
                    ctx.moveTo(particles[a].x, particles[a].y);
                    ctx.lineTo(particles[b].x, particles[b].y);
                    ctx.stroke();
                }
            }
        }
    }

    function animate() {
        ctx.clearRect(0, 0, canvas.width, canvas.height);
        particles.forEach(p => { p.update(); p.draw(); });
        connectParticles();
        requestAnimationFrame(animate);
    }

    window.addEventListener('resize', () => { resizeCanvas(); initParticles(); });
    window.addEventListener('mousemove', e => { mouse.x = e.clientX; mouse.y = e.clientY; });
    canvas.addEventListener('touchmove', e => { e.preventDefault(); mouse.x = e.touches[0].clientX; mouse.y = e.touches[0].clientY; }, { passive: false });
    window.addEventListener('touchend', () => { mouse.x = mouse.y = null; });

    resizeCanvas();
    initParticles();
    animate();

    // Enhanced page loading logic with fallbacks
    let loadingTimedOut = false;
    
    // Fallback timer in case load event doesn't fire
    const loadingFallbackTimer = setTimeout(() => {
        loadingTimedOut = true;
        console.log('Loading fallback triggered');
        preloader.classList.add('hidden');
        document.body.classList.add('loaded');
    }, 5000);
    
    // Normal load event handler
    window.addEventListener('load', () => {
        if (!loadingTimedOut) {
            clearTimeout(loadingFallbackTimer);
            setTimeout(() => {
                preloader.classList.add('hidden');
                document.body.classList.add('loaded');
            }, 3000);
        }
    });
    
    // DOMContentLoaded as backup (will fire earlier than load)
    document.addEventListener('DOMContentLoaded', () => {
        if (!loadingTimedOut) {
            setTimeout(() => {
                preloader.classList.add('hidden');
                document.body.classList.add('loaded');
            }, 4000);
        }
    });

    // Main functionality
    const currentLangSpan = document.getElementById('currentLang');
    const swapLangTargetSpan = document.getElementById('swapLangTarget');
    const inputText = document.getElementById('inputText');
    const outputText = document.getElementById('outputText');
    const translatorLoader = document.getElementById('translatorLoader');
    const mcQuestion = document.getElementById('mcQuestion');
    const mcOptions = document.getElementById('mcOptions');
    const mcLangSelect = document.getElementById('mcLang');
    const mcResult = document.getElementById('mcResult');
    const mcLoader = document.getElementById('mcLoader');
    const fillQuestion = document.getElementById('fillQuestion');
    const fillQuestionLangSelect = document.getElementById('fillQuestionLang');
    const fillOutputLangSelect = document.getElementById('fillOutputLang');
    const fillResult = document.getElementById('fillResult');
    const fillLoader = document.getElementById('fillLoader');

    function showLoader(el, show) { if (el) el.style.display = show ? 'block' : 'none'; }
    function shake(el) { el.classList.remove('shake'); void el.offsetWidth; el.classList.add('shake'); }
    function updateOutput(el, msg, isError=false) {
        el.textContent = msg;
        el.classList.add('has-content');
        if (isError) { el.style.color = currentTheme==='dark'? '#ff8a8a':'#dc3545'; el.style.fontWeight = 'bold'; }
        else { el.style.color = ''; el.style.fontWeight = ''; }
    }
    function clearOutput(el, placeholder) {
        el.textContent = placeholder;
        el.classList.remove('has-content');
        el.style.color = '';
        el.style.fontWeight = '';
    }

    function swapLanguage() {
        currentTranslateLang = currentTranslateLang === 'id' ? 'dyk' : 'id';
        let newName = currentTranslateLang==='id'?'Bahasa Indonesia':'Dayak Kenyah';
        let targetName = currentTranslateLang==='id'?'Dayak Kenyah':'Bahasa Indonesia';
        currentLangSpan.textContent = newName;
        swapLangTargetSpan.textContent = targetName;
        inputText.placeholder = `Tulis teks dalam ${newName}...`;
        inputText.value = '';
        clearOutput(outputText, 'Hasil terjemahan akan muncul di sini.');
    }

    let translateTimeout;
    inputText.addEventListener('input', () => {
        clearTimeout(translateTimeout);
        let val = inputText.value.trim();
        if (!val) { showLoader(translatorLoader, false); clearOutput(outputText, 'Hasil terjemahan akan muncul di sini.'); return; }
        showLoader(translatorLoader, true);
        clearOutput(outputText, '');
        translateTimeout = setTimeout(() => {
            fetch(`/translate?text=${encodeURIComponent(val)}&lang=${currentTranslateLang}`)
                .then(r => {
                    if (!r.ok) throw new Error(`HTTP error! Status: ${r.status}`);
                    return r.text();
                })
                .then(data => { 
                    showLoader(translatorLoader, false); 
                    updateOutput(outputText, data); 
                })
                .catch(err => { 
                    console.error('Translation error:', err);
                    showLoader(translatorLoader, false); 
                    updateOutput(outputText, 'Error saat menerjemahkan teks. Coba lagi.', true); 
                });
        }, 600);
    });

    function submitMC() {
        let q = mcQuestion.value.trim();
        let opts = mcOptions.value.trim();
        let lang = mcLangSelect.value;
        clearOutput(mcResult, '');
        if (!q || !opts) {
            if (!q) shake(mcQuestion);
            if (!opts) shake(mcOptions);
            updateOutput(mcResult, 'Kesalahan: Pertanyaan dan pilihan jawaban tidak boleh kosong!', true);
            return;
        }
        showLoader(mcLoader, true);
        fetch(`/mc?question=${encodeURIComponent(q)}&options=${encodeURIComponent(opts)}&lang=${lang}`)
            .then(r => {
                if (!r.ok) throw new Error(`HTTP error! Status: ${r.status}`);
                return r.text();
            })
            .then(data => { 
                showLoader(mcLoader, false); 
                updateOutput(mcResult, data); 
            })
            .catch(err => { 
                console.error('MC processing error:', err);
                showLoader(mcLoader, false); 
                updateOutput(mcResult, 'Error saat memproses soal pilihan ganda.', true); 
            });
    }

    function submitFill() {
        let q = fillQuestion.value.trim();
        let qLang = fillQuestionLangSelect.value;
        let oLang = fillOutputLangSelect.value;
        clearOutput(fillResult, '');
        if (!q) {
            shake(fillQuestion);
            updateOutput(fillResult, 'Kesalahan: Soal isian tidak boleh kosong!', true);
            return;
        }
        if (qLang === oLang) {
            shake(fillQuestionLangSelect);
            shake(fillOutputLangSelect);
            updateOutput(fillResult, 'Kesalahan: Bahasa soal dan bahasa jawaban tidak boleh sama!', true);
            return;
        }
        showLoader(fillLoader, true);
        fetch(`/fill?question=${encodeURIComponent(q)}&questionLang=${qLang}&outputLang=${oLang}`)
            .then(r => {
                if (!r.ok) throw new Error(`HTTP error! Status: ${r.status}`);
                return r.text();
            })
            .then(data => { 
                showLoader(fillLoader, false); 
                updateOutput(fillResult, data); 
            })
            .catch(err => { 
                console.error('Fill processing error:', err);
                showLoader(fillLoader, false); 
                updateOutput(fillResult, 'Error saat memproses soal isian.', true); 
            });
    }
    </script>
</body>
</html>


)rawliteral";

// Function declarations
void handleRoot();
void handleTranslate();
void handleMC();
void handleFill();
String extractQuotedText(const String& text);
void updateClientActivity(IPAddress clientIP);
void cleanupInactiveClients();

// BAGIAN ENGINE
bool refinedPartialMatch(const String& token, const String& dictWord) {
    String tokenLower = token;
    tokenLower.toLowerCase();
    String dictWordLower = dictWord;
    dictWordLower.toLowerCase();
    return tokenLower == dictWordLower;
}

std::vector<String> tokenizeSentence(const String& sentence) {
    String tmp = sentence;
    tmp.replace("?", " ");
    tmp.replace("!", " ");
    tmp.replace(".", " ");
    tmp.replace(",", " ");
    tmp.replace(":", " ");
    tmp.replace(";", " ");
    tmp.trim();

    std::vector<String> tokens;
    int start = 0;
    while (true) {
        int spacePos = tmp.indexOf(' ', start);
        if (spacePos == -1) {
            String last = tmp.substring(start);
            last.trim();
            if (last.length() > 0) tokens.push_back(last);
            break;
        } else {
            String piece = tmp.substring(start, spacePos);
            piece.trim();
            if (piece.length() > 0) tokens.push_back(piece);
            start = spacePos + 1;
        }
    }
    return tokens;
}

String translateSentencePartial(const String& input, const String& lang, const JsonObject& dictObj) {
    std::vector<String> tokens = tokenizeSentence(input);
    String result;
    
    // Set a maximum number of iterations to prevent potential hangs
    const int MAX_DICT_ITERATIONS = 1000;

    if (lang == "id") { // Indonesian -> Dayak
        for (auto &t : tokens) {
            String tLower = t;
            tLower.toLowerCase();
            bool found = false;
            int iterations = 0;
            
            for (JsonPair pair : dictObj) {
                String key = pair.key().c_str();
                String keyLower = key;
                keyLower.toLowerCase();
                
                if (tLower == keyLower) {
                    result += pair.value().as<String>() + " ";
                    found = true;
                    break;
                }
                
                iterations++;
                if (iterations >= MAX_DICT_ITERATIONS) {
                    break;
                }
            }
            
            if (!found) {
                result += t + " ";
            }
        }
    } else { // Dayak -> Indonesian
        for (auto &t : tokens) {
            String tLower = t;
            tLower.toLowerCase();
            bool found = false;
            int iterations = 0;
            
            for (JsonPair pair : dictObj) {
                String val = pair.value().as<String>();
                String valLower = val;
                valLower.toLowerCase();
                
                if (tLower == valLower) {
                    result += String(pair.key().c_str()) + " ";
                    found = true;
                    break;
                }
                
                iterations++;
                if (iterations >= MAX_DICT_ITERATIONS) {
                    break;
                }
            }
            
            if (!found) {
                result += t + " ";
            }
        }
    }
    
    result.trim();
    return result;
}

String doTranslation(const String& question, const String& questionLang, const String& outputLang, const JsonObject& dictObj) {
    if (questionLang == outputLang) {
        return question;
    }
    return translateSentencePartial(question, questionLang, dictObj);
}

// SETUP & ROUTING
void setup() {
    Serial.begin(115200);
    Serial.println("\nDayak Kenyah Translator Starting...");
    
    // Configure AP mode with optimized settings
    WiFi.disconnect();  // Disconnect any existing connection
    delay(100);
    
    WiFi.mode(WIFI_AP);
    WiFi.setAutoReconnect(true);
    
    // Set static IP configuration
    if (!WiFi.softAPConfig(AP_IP, AP_IP, AP_SUBNET)) {
        Serial.println("AP Configuration Failed!");
    }
    
    // Start the access point with the specified settings
    if (!WiFi.softAP(AP_SSID, AP_PASSWORD)) {
        Serial.println("AP Setup Failed!");
    } else {
        Serial.println("AP Mode Configured Successfully");
        Serial.print("Access Point Name: ");
        Serial.println(AP_SSID);
        Serial.print("AP IP address: ");
        Serial.println(WiFi.softAPIP());
    }
    
    // Setup DNS for captive portal (wildcard mapping)
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    if (!dnsServer.start(DNS_PORT, "*", AP_IP)) {
        Serial.println("DNS Server Failed to Start!");
    } else {
        Serial.println("DNS Server Started");
    }

    // Start mDNS responder for dayak.local (for devices that support mDNS)
    if (!MDNS.begin("dayak")) {
        Serial.println("Error setting up mDNS responder!");
    } else {
        MDNS.addService("http", "tcp", 80);
        Serial.println("mDNS responder started: http://dayak.local");
    }

    // Parse the dictionary JSON
    Serial.println("Loading dictionary...");
    DeserializationError error = deserializeJson(dict, dictionary_json);
    if (error) {
        Serial.println("Error parsing JSON: " + String(error.c_str()));
        return;
    }
    Serial.println("Dictionary loaded successfully");

    // Setup server routes
    server.on("/", HTTP_GET, handleRoot);
    server.on("/translate", HTTP_GET, handleTranslate);
    server.on("/mc", HTTP_GET, handleMC);
    server.on("/fill", HTTP_GET, handleFill);
    
    // Add a status endpoint to check server health
    server.on("/status", HTTP_GET, []() {
        String status = "Server is running. Uptime: " + String(millis() / 1000) + "s";
        server.send(200, "text/plain", status);
    });
    
    // Detect captive portal requests
    server.on("/generate_204", HTTP_GET, []() { // Android/Chrome OS captive portal check
        server.sendHeader("Location", "http://" + WiFi.softAPIP().toString());
        server.send(302, "text/plain", "");
    });
    server.on("/mobile/status.php", HTTP_GET, []() { // Android fallback captive portal check
        server.send(200, "text/html", "");
    });
    server.on("/hotspot-detect.html", HTTP_GET, []() { // iOS captive portal check
        server.send_P(200, "text/html", index_html);
    });
    server.on("/library/test/success.html", HTTP_GET, []() { // iOS captive portal check
        server.send_P(200, "text/html", index_html);
    });
    server.on("/success.txt", HTTP_GET, []() { // iOS captive portal check
        server.send(200, "text/plain", "success");
    });
    server.on("/ncsi.txt", HTTP_GET, []() { // Windows captive portal check
        server.send(200, "text/plain", "Microsoft NCSI");
    });
    
    // Handler default untuk semua path yang tidak dikenali
    server.onNotFound([]() {
        server.send_P(200, "text/html", index_html);
    });
    
    // Start the web server
    server.begin();
    Serial.println("HTTP Server Started");
    Serial.println("Dayak Kenyah Translator Ready!");
}

void loop() {
    // Process DNS requests to ensure captive portal works correctly
    dnsServer.processNextRequest();
    
    // Handle client requests with a small delay to prevent CPU hogging
    server.handleClient();
    
    // Periodically clean up inactive clients (every 30 seconds)
    static unsigned long lastCleanupTime = 0;
    unsigned long currentMillis = millis();
    if (currentMillis - lastCleanupTime > 30000) { // 30 seconds
        cleanupInactiveClients();
        lastCleanupTime = currentMillis;
    }
    
    // Small delay to prevent watchdog timer issues
    delay(1);
}

// HANDLER UTAMA (Root)
void handleRoot() {
    updateClientActivity(server.client().remoteIP());
    server.send_P(200, "text/html", index_html);
}

// HANDLER TERJEMAHAN
void handleTranslate() {
    updateClientActivity(server.client().remoteIP());
    if (!server.hasArg("text") || !server.hasArg("lang")) {
        server.send(400, "text/plain", "Parameter tidak lengkap");
        return;
    }
    String text = server.arg("text");
    String lang = server.arg("lang");
    String translated = translateSentencePartial(text, lang, dict.as<JsonObject>());
    server.send(200, "text/plain", translated);
}

// HANDLER MULTIPLE CHOICE (MC)
void handleMC() {
    updateClientActivity(server.client().remoteIP());
    if (!server.hasArg("question") || !server.hasArg("options") || !server.hasArg("lang")) {
        server.send(400, "text/plain", "Parameter tidak lengkap untuk soal pilihan ganda");
        return;
    }
    String question = server.arg("question");
    String optionsStr = server.arg("options");
    String lang = server.arg("lang");

    String extractedText = extractQuotedText(question);
    if (extractedText.length() == 0) {
        server.send(200, "text/plain", "Tidak ditemukan teks dalam tanda kutip atau tanda kurung.");
        return;
    }

    String fromLang = (lang == "id_to_dyk") ? "id" : "dyk";
    String translated = translateSentencePartial(extractedText, fromLang, dict.as<JsonObject>());
    translated.trim();
    translated.toLowerCase();

    const int maxOptions = 10;
    String options[maxOptions];
    int count = 0;
    int pos = 0;
    while (pos >= 0 && count < maxOptions) {
        int commaPos = optionsStr.indexOf(',', pos);
        if (commaPos == -1) {
            options[count++] = optionsStr.substring(pos);
            break;
        } else {
            options[count++] = optionsStr.substring(pos, commaPos);
            pos = commaPos + 1;
        }
    }
    for (int i = 0; i < count; i++) {
        options[i].trim();
        options[i].toLowerCase();
    }

    char answerLetter = '\0';
    const char letters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    for (int i = 0; i < count; i++) {
        if (options[i] == translated) {
            answerLetter = letters[i];
            break;
        }
    }

    if (answerLetter == '\0') {
        server.send(200, "text/plain", "Jawaban yang benar tidak ditemukan dalam pilihan yang diberikan.");
    } else {
        String response = "Jawaban yang benar kemungkinan: ";
        response += answerLetter;
        response += ") ";
        response += options[answerLetter - 'A'];
        server.send(200, "text/plain", response);
    }
}

// HANDLER FILL-IN ANSWER (Soal Isian)
void handleFill() {
    updateClientActivity(server.client().remoteIP());
    if (!server.hasArg("question") || !server.hasArg("questionLang") || !server.hasArg("outputLang")) {
        server.send(400, "text/plain", "Parameter tidak lengkap untuk soal isian");
        return;
    }
    String question = server.arg("question");
    String qLang = server.arg("questionLang");
    String oLang = server.arg("outputLang");

    if ((qLang != "id" && qLang != "dyk") || (oLang != "id" && oLang != "dyk")) {
        server.send(400, "text/plain", "Parameter questionLang atau outputLang tidak valid. Gunakan 'id' atau 'dyk'.");
        return;
    }

    String extractedText = extractQuotedText(question);
    if (extractedText.length() == 0) {
        server.send(200, "text/plain", "Tidak ditemukan teks dalam tanda kutip atau tanda kurung.");
        return;
    }

    String answer = doTranslation(extractedText, qLang, oLang, dict.as<JsonObject>());
    answer.trim();

    if (answer.length() == 0) {
        server.send(200, "text/plain", "Tidak ditemukan kosakata yang sesuai. Periksa kembali soal Anda.");
        return;
    }
    
    String response = "Jawaban isian: " + answer;
    server.send(200, "text/plain", response);
}

// Fungsi extractQuotedText
String extractQuotedText(const String& text) {
    int startQuote = text.indexOf('"');
    int endQuote = -1;
    if (startQuote != -1) {
        endQuote = text.indexOf('"', startQuote + 1);
        if (endQuote != -1) {
            return text.substring(startQuote + 1, endQuote);
        }
    }
    
    int startParen = text.indexOf('(');
    int endParen = -1;
    if (startParen != -1) {
        endParen = text.indexOf(')', startParen + 1);
        if (endParen != -1) {
            return text.substring(startParen + 1, endParen);
        }
    }
    
    return "";
}

void updateClientActivity(IPAddress clientIP) {
    bool found = false;
    for (auto& client : activeClients) {
        if (client.ip == clientIP) {
            client.lastActivity = millis();
            found = true;
            break;
        }
    }
    if (!found && activeClients.size() < 50) { // Limit to 50 concurrent clients
        ClientInfo newClient = {clientIP, millis()};
        activeClients.push_back(newClient);
    }
}

void cleanupInactiveClients() {
    unsigned long currentTime = millis();
    for (auto it = activeClients.begin(); it != activeClients.end();) {
        if (currentTime - it->lastActivity > CLIENT_TIMEOUT) {
            it = activeClients.erase(it);
        } else {
            ++it;
        }
    }
}