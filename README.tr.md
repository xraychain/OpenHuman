# OpenHuman

**Yerel bilgisayarlarda çalışan, durumunu sürdüren bir dijital organizma geliştirmeye yönelik açık kaynak araştırma girişimi.**

[English](README.md) · [Türkçe](README.tr.md)

> **Proje durumu: Deneysel sentetik doku v0.1.** OpenHuman henüz çalışan bir insan simülatörü değildir. Biyolojik tahmin veya klinik kullanım yeteneği olduğu iddia edilmemektedir.


## Çalışan v0.1 prototip

Kullanıcının sağladığı C++20 taslak artık derlenmiş ve yerel deneylerle sınanmış bir 2B sentetik doku simülasyonudur. İnsan fizyolojisiyle kalibre edilmemiştir. Aşağıdaki uzun vadeli vizyonun yalnızca ilk deneysel katmanı uygulanmıştır.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
./build/openhuman --steps 30000 --metrics baseline.csv --save tissue.ckpt
./build/openhuman --resume tissue.ckpt --steps 1000
python3 experiments/run_all.py build/openhuman
python3 experiments/benchmark.py build/openhuman
```

Gereksinimler: C++20 derleyici, CMake >= 3.16 ve Python 3.9+. Python betikleri yalnızca standart kütüphaneyi kullanır. OpenMP isteğe bağlıdır (`-DOPENHUMAN_OPENMP=ON`, ayrıca OpenMP geliştirme kitaplığı gerekir). `--quick` kısa CI deneyi içindir. Sonuçlar varsayılan olarak `results/` klasörüne yazılır.

Ölçülmüş sonuçlar: [araştırma bulguları](docs/RESEARCH_FINDINGS.md). Ayrıca [mimari](docs/ARCHITECTURE.md), [test kapsamı](docs/VALIDATION.md), [yol haritası](docs/ROADMAP.md) ve [API bütçe sınırı](docs/API_BUDGET.md). GitHub CI yapılandırması eklendi; uzaktaki CI henüz çalıştırılmadı. Depodaki MIT lisansı geçerlidir.

## Vizyon

Bir bilgisayarın içinde, **değişen iç durumunu koruyan**, çevresine tepki veren ve simülasyon zamanı boyunca varlığını sürdüren bir organizma oluşturabilir miyiz?

OpenHuman bu soruyu araştırmayı amaçlıyor. Hedef; birbirinden bağımsız sağlık puanları hesaplayan modüller yerine, hücrelerin, dokuların, organların ve düzenleyici süreçlerin zaman içinde etkileştiği bir **hesaplamalı organizmanın** temellerini kurmak.

Uzun vadeli amaç, yaş, cinsiyete bağlı fizyolojik özellikler, vücut yapısı, yaşanılan çevre, beslenme, fiziksel aktivite ve mevcut sağlık ölçümleri gibi verilerden sanal bir organizmanın başlangıç durumunu oluşturmak ve farklı koşullarda bu organizmanın nasıl değiştiğini incelemek.

Bu, mevcut tıbbi kayıtlarla gerçek bir insanın bugün birebir yeniden üretilebildiği iddiası **değildir**. Profil verileri eksiktir; oluşturulacak iç durum kaçınılmaz olarak varsayımlar ve belirsizlikler içerecektir.

## Temel fikir: zaman içinde değişen bir sistem

OpenHuman birbirine bağlı şu yetenekleri araştırmayı hedefler:

- **Organism Genesis:** Yapılandırılabilir bir profilden, belirsizlik aralıkları açıkça tanımlanmış olası bir başlangıç durumu üretmek.
- **Kalıcı durum:** Organizmanın simülasyon durumunu sürdürmek, kaydetmek ve uygulama yeniden açıldığında devam ettirmek.
- **Çok ölçekli biyoloji:** Hücre ajanlarını veya hücre topluluklarını, dokuları, organ sistemlerini ve bunların ortak çevrelerini ilişkilendirmek.
- **Ortaya çıkan davranış:** Her senaryonun sonucunu önceden yazmak yerine, etkileşimlerin, kaynak kısıtlarının ve geri bildirim döngülerinin model sonuçlarını oluşturmasını sağlamak.
- **Homeostaz ve adaptasyon:** Enerji, oksijen, besin, atık ve diğer modellenmiş değişkenlerde denge kurma süreçlerini araştırmak.
- **Hastalık ve müdahale deneyleri:** Tanımlı bozucu etkiler veya eklenti modelleri uygulayarak simülasyon içindeki değişimleri gözlemek.
- **İzlenebilir deneyler:** Olayları, model sürümlerini, girdileri, durum değişikliklerini ve belirsizlikleri kaydederek sonuçları tekrarlanabilir kılmak.
- **Hızlandırılmış simülasyon zamanı:** Model doğruluğu ve donanımın izin verdiği koşullarda gerçek zamandan hızlı çalışmayı araştırmak.

Kanser **olası araştırma senaryolarından biridir**; projenin temeli değildir. Asıl odak simüle edilen organizmanın kendisidir.

## Araştırma ilkeleri

1. **Görünümden önce biyoloji.** Etkileyici bir 3D insan modeli, biyolojik doğruluğun kanıtı değildir.
2. **Ortak kurallar, farklı durumlar.** Milyonlarca sanal ajan ortak öğrenilmiş veya mekanistik kuralları kullanırken farklı yerel durumlara sahip olabilir. Model parametreleri ile simülasyon durumunun bellek ihtiyacı aynı şey değildir.
3. **Birden fazla ölçek, açık varsayımlar.** Tipik kişisel bilgisayarlarda bütün insan vücudunun her hücresini moleküler ayrıntıda simüle etmek bugün gerçekçi değildir. Çözünürlük bilimsel soruya göre seçilmelidir.
4. **Fiziksel ve biyolojik kısıtlar.** Öğrenilmiş modeller; korunum yasaları, bilinen fizyoloji ve deneysel gözlemlerle sınanmalıdır.
5. **Belirsizlik birinci sınıf çıktıdır.** Benzer insan profilleri aynı organizmaya veya aynı geleceğe karşılık gelmek zorunda değildir.
6. **Gösterişten önce tekrarlanabilirlik.** Her deney rastgelelik tohumunu, girdilerini, model sürümünü, varsayımlarını ve değerlendirme ölçütlerini kaydetmelidir.
7. **Önce yerellik ve gizlilik.** Simülasyon ve hassas veriler varsayılan olarak kullanıcının bilgisayarında kalmalıdır. Gelecekte uygun biçimde kimliksizleştirilmiş sonuçların paylaşımı yalnızca açık tercihle yapılmalıdır.
8. **Bilimsel doğrulama zorunlu.** Kendi içinde tutarlı bir simülasyon, klinik tahmin doğruluğunu kanıtlamaz.

## Önerilen mimari

```text
                   İnsan profili / isteğe bağlı ölçümler
                                  |
                                  v
                       Organism Genesis Engine
                                  |
                                  v
                       Kalıcı Organizma Durumu
                                  |
             +--------------------+--------------------+
             |                    |                    |
             v                    v                    v
       Hücre / ajan         Doku ve organ          Düzenleme ve
         süreçleri             ağları                çevre
             |                    |                    |
             +--------------------+--------------------+
                                  |
                                  v
                         Simulation Runtime
                    (yerel CPU / GPU üzerinde)
                                  |
                   +--------------+-------------+
                   |                            |
                   v                            v
           Durum kayıtları              Deney analizi
             ve günlükler             (varsayılan: yerel)
                                                 |
                                                 v
                                    İsteğe bağlı, açık izinli
                                    kimliksiz sonuç paylaşımı
```

Bu şema **öneridir**; uygulanmış bir mimari değildir. Makine öğrenmesi bazı bileşenleri destekleyebilir, ancak bir LLM tek başına doğrulanmış insan fizyolojisi modeli değildir.

## Yol haritası

Aşağıdaki tablo uzun vadeli araştırma programıdır. 0–2 ve 6. aşamalar için ilk kafes prototipi uygulanmıştır; biyolojik doğrulama henüz yapılmamıştır.

| Aşama | Araştırma hedefi | Örnek başarı ölçütü |
| --- | --- | --- |
| 0 | Model sınırlarını, veri arayüzlerini ve kanıt ölçütlerini tanımlamak | Yayımlanmış teknik tasarım ve test planı |
| 1 | Küçük bir sentetik doku modeli geliştirmek | Besin, enerji tüketimi, atık, büyüme ve hücre ölümü arasında kararlı ve tekrarlanabilir etkileşim |
| 2 | Geri bildirim ve bozucu etki deneylerini eklemek | Kontrollü koşullarda ölçülebilir toparlanma veya bozulma |
| 3 | Birden fazla biyolojik alt sistemi birleştirmek | Model sınırlarında korunum ve arayüz testleri |
| 4 | Profile göre organizma oluşturmayı denemek | Yanıltıcı kesinlikte tek hasta yerine makul olası durum kümeleri üretmek |
| 5 | Bağımsız yayımlanmış veya uygun izinlerle yönetilen verilerle karşılaştırmak | Belgelenmiş kalibrasyon, belirsizlik ve görülmemiş veride hata ölçümleri |
| 6 | Yerel yürütmeyi ve zaman hızlandırmasını optimize etmek | Yayımlanmış hız–model doğruluğu testleri |

Gelecekteki hedeflerden biri, belirli model çözünürlüklerinde **aylar veya yıllar süren simülasyon zamanının tüketici bilgisayarlarında saatler veya günler içinde çalıştırılıp çalıştırılamayacağını** araştırmaktır. Bu bir mühendislik hedefidir; **performans veya doğruluk vaadi değildir**.

## OpenHuman ne değildir?

- Bir tıbbi cihaz, tanı sistemi veya klinik karar destek aracı değildir.
- Kimliği belirli bir insanın dijital kopyası değildir.
- Bireysel hastalık seyri, sağkalım veya tedavi yanıtını güvenilir biçimde tahmin ettiği kanıtlanmış bir sistem değildir.
- Simülasyonda ortaya çıkan davranışın biyolojik yaşam veya bilinç olduğu iddiasını taşımaz.
- Klinik araştırmaların, sağlık profesyonellerinin veya deneysel doğrulamanın yerine geçmez.

**OpenHuman çıktılarıyla tıbbi karar verilmemelidir.** Gelecekte hasta verileri araştırma amacıyla kullanılacaksa gerekli izin, gizlilik, etik kurul ve yasal gereklilikler karşılanmalıdır.

## Katkıda bulunmak

İlk kafes prototipi kullanılabilir. Katkılar için en değerli alanlar:

- Konuyla ilgili hakemli araştırmalar ve mevcut açık kaynak simülasyon altyapıları.
- Test edilebilir model tasarımları, karşılaştırma testleri ve uygun kullanım haklarına sahip doğrulama verileri.
- Yapılabilirlik, ölçekleme, biyolojik varsayımlar ve mimari üzerine teknik eleştiriler.

Değişiklik önermeden önce testleri çalıştırın; model ve performans iddialarına ölçüm ekleyin.

## Lisans

Proje [MIT Lisansı](LICENSE) altında dağıtılmaktadır.

---

*OpenHuman uzun vadeli ve iddialı bir araştırma yönüdür. İlk hedefimiz ise küçük ve sınanabilir: durumunu sürdürebilen, dış etkilere tepki veren ve davranışı bilimsel verilerle değerlendirilebilen küçük bir sanal biyolojik sistem oluşturmak.*
