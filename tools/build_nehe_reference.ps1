param(
    [string[]]$Lessons = @("1","2","3","4","5","6","7","8","9","10","11","12")
)

$ErrorActionPreference = "Stop"

$repo = Resolve-Path (Join-Path $PSScriptRoot "..")
$srcRoot = Join-Path $repo "dist\nehe_reference\src"
$buildRoot = Join-Path $repo "dist\nehe_reference\build"
$bash = "C:\devkitPro\msys2\usr\bin\bash.exe"

if (-not (Test-Path $bash)) {
    throw "MSYS bash not found at $bash"
}

New-Item -ItemType Directory -Force -Path $buildRoot | Out-Null

$glauxCompat = @'
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glaux.h>

static AUX_RGBImageRec *load_bmp_rgb(const char *filename)
{
    FILE *f = fopen(filename, "rb");
    if (!f) {
        return NULL;
    }

    unsigned char fileHeader[14];
    unsigned char infoHeader[40];
    if (fread(fileHeader, 1, sizeof(fileHeader), f) != sizeof(fileHeader) ||
        fread(infoHeader, 1, sizeof(infoHeader), f) != sizeof(infoHeader)) {
        fclose(f);
        return NULL;
    }

    if (fileHeader[0] != 'B' || fileHeader[1] != 'M') {
        fclose(f);
        return NULL;
    }

    unsigned int dataOffset = (unsigned int)fileHeader[10] |
        ((unsigned int)fileHeader[11] << 8) |
        ((unsigned int)fileHeader[12] << 16) |
        ((unsigned int)fileHeader[13] << 24);
    int width = (int)((unsigned int)infoHeader[4] |
        ((unsigned int)infoHeader[5] << 8) |
        ((unsigned int)infoHeader[6] << 16) |
        ((unsigned int)infoHeader[7] << 24));
    int height = (int)((unsigned int)infoHeader[8] |
        ((unsigned int)infoHeader[9] << 8) |
        ((unsigned int)infoHeader[10] << 16) |
        ((unsigned int)infoHeader[11] << 24));
    unsigned short planes = (unsigned short)((unsigned int)infoHeader[12] |
        ((unsigned int)infoHeader[13] << 8));
    unsigned short bits = (unsigned short)((unsigned int)infoHeader[14] |
        ((unsigned int)infoHeader[15] << 8));
    unsigned int compression = (unsigned int)infoHeader[16] |
        ((unsigned int)infoHeader[17] << 8) |
        ((unsigned int)infoHeader[18] << 16) |
        ((unsigned int)infoHeader[19] << 24);

    if (width <= 0 || height == 0 || planes != 1 || (bits != 24 && bits != 8) || compression != 0) {
        fclose(f);
        return NULL;
    }

    int topDown = height < 0;
    int absHeight = topDown ? -height : height;
    unsigned int rowStride = bits == 24
        ? (unsigned int)(((width * 3) + 3) & ~3)
        : (unsigned int)((width + 3) & ~3);
    unsigned int paletteCount = 0;
    unsigned char palette[256][4];
    if (bits == 8) {
        paletteCount = (unsigned int)infoHeader[32] |
            ((unsigned int)infoHeader[33] << 8) |
            ((unsigned int)infoHeader[34] << 16) |
            ((unsigned int)infoHeader[35] << 24);
        if (paletteCount == 0 || paletteCount > 256) {
            paletteCount = 256;
        }
        if (fread(palette, 4, paletteCount, f) != paletteCount) {
            fclose(f);
            return NULL;
        }
    }
    unsigned char *row = (unsigned char *)malloc(rowStride);
    AUX_RGBImageRec *image = (AUX_RGBImageRec *)calloc(1, sizeof(AUX_RGBImageRec));
    unsigned char *rgb = (unsigned char *)malloc((size_t)width * (size_t)absHeight * 3u);
    if (!row || !image || !rgb) {
        free(row);
        free(image);
        free(rgb);
        fclose(f);
        return NULL;
    }

    fseek(f, (long)dataOffset, SEEK_SET);
    for (int y = 0; y < absHeight; ++y) {
        int dstY = topDown ? y : (absHeight - 1 - y);
        if (fread(row, 1, rowStride, f) != rowStride) {
            free(row);
            free(image);
            free(rgb);
            fclose(f);
            return NULL;
        }
        for (int x = 0; x < width; ++x) {
            unsigned char *dst = rgb + ((dstY * width + x) * 3);
            if (bits == 24) {
                unsigned char *src = row + x * 3;
                dst[0] = src[2];
                dst[1] = src[1];
                dst[2] = src[0];
            } else {
                unsigned int index = row[x];
                if (index >= paletteCount) {
                    index = 0;
                }
                dst[0] = palette[index][2];
                dst[1] = palette[index][1];
                dst[2] = palette[index][0];
            }
        }
    }

    free(row);
    fclose(f);
    image->sizeX = width;
    image->sizeY = absHeight;
    image->data = rgb;
    return image;
}

extern "C" AUX_RGBImageRec *APIENTRY auxDIBImageLoadA(LPCSTR filename)
{
    return load_bmp_rgb(filename);
}

extern "C" AUX_RGBImageRec *APIENTRY auxRGBImageLoadA(LPCSTR filename)
{
    return load_bmp_rgb(filename);
}

extern "C" AUX_RGBImageRec *APIENTRY auxDIBImageLoadW(LPCWSTR filename)
{
    char path[MAX_PATH];
    WideCharToMultiByte(CP_ACP, 0, filename, -1, path, MAX_PATH, NULL, NULL);
    return load_bmp_rgb(path);
}

extern "C" AUX_RGBImageRec *APIENTRY auxRGBImageLoadW(LPCWSTR filename)
{
    return auxDIBImageLoadW(filename);
}
'@

foreach ($lessonToken in $Lessons) {
    foreach ($lessonPart in ($lessonToken -split ",")) {
        if ([string]::IsNullOrWhiteSpace($lessonPart)) {
            continue
        }
        $lesson = [int]$lessonPart.Trim()
        if ($lesson -lt 1 -or $lesson -gt 12) {
            throw "Unsupported lesson number: $lesson"
        }
    $name = "lesson$lesson"
    $src = Join-Path $srcRoot $name
    if (-not (Test-Path $src)) {
        throw "Missing source directory: $src"
    }

    $dst = Join-Path $buildRoot $name
    if (Test-Path $dst) {
        $resolvedDst = (Resolve-Path $dst).Path
        $resolvedBuildRoot = (Resolve-Path $buildRoot).Path
        if (-not $resolvedDst.StartsWith($resolvedBuildRoot, [StringComparison]::OrdinalIgnoreCase)) {
            throw "Refusing to delete unexpected path: $resolvedDst"
        }
        Remove-Item -LiteralPath $dst -Recurse -Force
    }

    Copy-Item -LiteralPath $src -Destination $dst -Recurse
    Set-Content -LiteralPath (Join-Path $dst "glaux_compat.cpp") -Encoding ASCII -Value $glauxCompat

    $cppPath = Join-Path $dst "$name.cpp"
    $cpp = Get-Content -LiteralPath $cppPath -Raw
    $cpp = $cpp -replace '<gl\\gl\.h>', '<GL/gl.h>'
    $cpp = $cpp -replace '<gl\\glu\.h>', '<GL/glu.h>'
    $cpp = $cpp -replace '<gl\\glaux\.h>', '<GL/glaux.h>'
    $cpp = $cpp -replace 'const\s+num\s*=\s*50;', 'const int num=50;'
    $cpp = $cpp -replace 'ChangeDisplaySettings\(&dmScreenSettings, CDS_FULLSCREEN\);', '/* reference build stays windowed */'
    $cpp = $cpp -replace 'ChangeDisplaySettings\(NULL, 0\);', '/* reference build stays windowed */'
    $cpp = $cpp -replace '"Jeff Molofee''s GL Code Tutorial \.\.\. NeHe ''99"', ('"NeHe Reference Lesson {0:00}"' -f $lesson)
    Set-Content -LiteralPath $cppPath -Encoding ASCII -Value $cpp

    $msysDir = $dst -replace '\\','/'
    $msysDir = $msysDir -replace '^C:','/c'
    $cmd = "cd '$msysDir' && /usr/bin/g++ -std=gnu++98 -Wall -Wno-write-strings -Wno-unused-variable -Wno-unused-but-set-variable -mwindows -o ${name}.exe ${name}.cpp glaux_compat.cpp -lopengl32 -lglu32 -lgdi32 -luser32"
    & $bash -lc $cmd
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed for $name"
    }
    Write-Host "Built $name"
    }
}
