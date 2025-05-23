# Sisop-4-2025-IT31

- Erlangga Valdhio Putra Sulistio_5027241030
- Rayhan Agnan Kusuma_5027241102
- S. Farhan Baig_5027241097


# Soal 1_

## Deskripsi Soal
Shorekeeper menemukan teks hexadecimal anomali di Tethys' Deep. Tugas:
1. Ekstraksi file teks
2. Konversi hex ke gambar
3. Penamaan file dengan timestamp
4. Pencatatan log aktivitas.

---

### **1a: Ekstraksi File Anomali**
**Tujuan**:  
Mengambil file ZIP dari sumber, mengekstrak ke direktori `anomali`, dan menghapus file ZIP.

**Langkah**:
1. # Salin file ZIP ke direktori kerja
```bash
cp '/path/anomali.zip' ~/blackshores/
```
2. # Unzip dan hapus file
```bash
unzip anomali.zip -d anomali
rm anomali.zip
```

Struktur Hasil:
```
anomali/
├── 1.txt
├── 2.txt
├── 3.txt
├── 4.txt
├── 5.txt
├── 6.txt
└── 7.txt
```

### 1b: Konversi Hex ke Gambar
**Tujuan**
Mengubah string hexadecimal dalam file `.txt` menjadi file gambar.

Script Awal `(hex_to_image.sh)`:
```bash
#!/bin/bash
INPUT="anomali/1.txt"
OUTPUT="image/output.png"
HEXDATA=$(cat $INPUT | tr -d ' \n\r')
echo $HEXDATA | xxd -r -p > $OUTPUT
echo "File gambar berhasil dibuat di $OUTPUT"
```

**Cara Jalankan**
```bash
chmod +x hex_to_image.sh
./hex_to_image.sh
```

### 1c: Penamaan File dengan Timestamp
**Tujuan**
Memberi nama file dengan format:
`[nama_file]_image_[YYYY-mm-dd]_[HH-MM-SS].png`

Script (`hex_to_image_timestamp.sh`):
```bash
#!/bin/bash
mkdir -p image
for input_file in anomali/*.txt; do
    base_name=$(basename "$input_file" .txt)
    timestamp=$(date +"%Y-%m-%d_%H-%M-%S")
    output_file="image/${base_name}_image_${timestamp}.png"
    hex_data=$(cat "$input_file" | tr -d ' \n\r')
    echo "$hex_data" | xxd -r -p > "$output_file"
    echo "Berhasil mengonversi $base_name.txt → $(basename "$output_file")"
done
```
