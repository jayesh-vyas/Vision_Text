
const express = require("express");
const Tesseract = require("tesseract.js");
const cors = require("cors");
const bodyParser = require("body-parser");
const say = require("say");
const stringSimilarity = require("string-similarity");
const sharp = require("sharp");
const { Jimp } = require("jimp");
const fs = require("fs");
const path = require("path");

const app = express();
const port = process.env.PORT || 3000;

app.use(cors());
app.use(bodyParser.raw({ type: "image/jpeg", limit: "10mb" }));

let lastSpokenText = "";


function isGibberish(text) {
  if (!text.trim()) return true;
  const words = text.split(/\s+/);
  if (words.length < 3) return true;
  const gibberishPatterns = /[^a-zA-Z0-9 .,?!]/g;
  return (text.match(gibberishPatterns) || []).length > text.length * 0.3 || /(.)\1{3,}/g.test(text);
}


function isSimilarText(newText, oldText, threshold = 0.85) {
  if (!oldText) return false;
  const similarity = stringSimilarity.compareTwoStrings(newText, oldText);
  return similarity >= threshold;
}

app.get("/", (req, res) => res.json({ message: "OCR API is active." }));

app.post("/ocr", async (req, res) => {
  if (!req.body?.length) {
    return res.status(400).json({ error: "No image uploaded" });
  }

  const timestamp = Date.now();
  const rawImagePath = `temp/image_${timestamp}.jpg`;
  const preprocessedPath = `temp/image_${timestamp}_processed.jpg`;

  try {
    
    fs.writeFileSync(rawImagePath, req.body);

    await sharp(rawImagePath)
      .extract({ left: 187, top: 0, width: 545, height: 600 }) 
      .grayscale()
      .normalize() 
      .resize({ width: 1000 }) 
      .modulate({ brightness: 1.2, saturation: 1.1 }) 
      .linear(1.2, -30) 
      .toFile(preprocessedPath);

    const { data: { text } } = await Tesseract.recognize(preprocessedPath, "eng", {
      logger: info => console.log(`[OCR] ${info.status}`),
    });

    const cleanedText = text.replace(/\n/g, " ").trim();
    console.log("[OCR Result]:", cleanedText);

    if (isGibberish(cleanedText)) {
      console.log("[INFO] Gibberish detected, skipping TTS.");
      return res.json({ result: cleanedText, message: "OCR produced gibberish, not playing speech." });
    }

    if (isSimilarText(cleanedText, lastSpokenText)) {
      console.log("[INFO] Similar text detected, skipping TTS.");
      return res.json({ result: cleanedText, message: "Text is too similar to the previous one, not playing speech." });
    }

    say.stop();
    say.speak(cleanedText, "Alex", 1.0, (err) => {
      if (err) console.error("[TTS ERROR]", err);
      console.log("[INFO] Speech played.");
    });

    lastSpokenText = cleanedText;
    res.json({ result: cleanedText });
  } catch (error) {
    console.error("[ERROR]", error);
    res.status(500).json({ error: "OCR failed", details: error.message });
  } 
});

app.listen(port, () => console.log(`[SERVER] Running on port ${port}`));

