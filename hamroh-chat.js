// Local, rule-based hackathon chat parser. It does not claim to be an LLM.
// All amounts are UZS. No transfer or purchase is initiated.
const normalize = s => String(s || '').toLocaleLowerCase('uz-UZ').replace(/[‘’ʻ`´]/g, "'");

const expenseWords = /(?:xarajat|harajat|sarflad|sarflandi|sarfla|ishlatdim|to'ladim|toladim|ketdi|sotib oldim|chiqim|spent|расход|потратил|трата)/i;
const incomeWords = /(?:daromad|kirim|tushum|pul topdim|sotuvdan|tushdi|income|доход|выручка)/i;

// Deliberately allow only a single amount. 50 000, 50.000, 50 ming, 1.5 mln.
export function findMoney(text) {
  const s = normalize(text).replace(/\u00a0/g, ' ');
  const pattern = /(?:^|[^\w\d])([0-9]{1,3}(?:[ .,][0-9]{3})+|[0-9]+(?:[.,][0-9]{1,2})?)\s*(mln|million|ming|mingta|тыс|млн|k|m|so'm|som|sum|сум|сўм|uzs)?\b/giu;
  const amounts = [];
  for (const m of s.matchAll(pattern)) {
    const raw = m[1]; const unit = (m[2] || '').toLocaleLowerCase('uz-UZ');
    let n;
    if (/^(?:mln|million|млн|m)$/.test(unit)) n = Number(raw.replace(',', '.')) * 1_000_000;
    else if (/^(?:ming|mingta|тыс|k)$/.test(unit)) n = Number(raw.replace(',', '.')) * 1_000;
    else n = Number(raw.replace(/[ .,]/g, ''));
    if (Number.isSafeInteger(n) && n > 0 && n <= 1_000_000_000_000) amounts.push(n);
  }
  return amounts;
}

export function inferCategory(text) {
  const s = normalize(text);
  const categories = [
    [/ovqat|tushlik|nonushta|kechki|restoran|kafe|choy|food|еда/, 'Oziq-ovqat'],
    [/taksi|taxi|metro|avtobus|benzin|yo'l|yol|transport|такси/, 'Transport'],
    [/ijara|arenda|аренда/, 'Ijara'],
    [/internet|aloqa|telefon|sim karta|связь/, 'Aloqa'],
    [/reklama|target|marketing|реклама/, 'Reklama'],
    [/maosh|oylik|xodim|зарплата/, 'Maosh'],
    [/bozor|mahsulot|tovar|xarid|закупк/, 'Xarid'],
    [/elektr|svet|gaz|suv|коммунал/, 'Kommunal'],
  ];
  return categories.find(([p]) => p.test(s))?.[1] || 'Boshqa';
}

export function parseTejamChat(text) {
  const s = normalize(text).trim();
  if (!s) return {kind:'empty'};
  if (s.length > 500) return {kind:'invalid', reason:'Xabar 500 belgidan oshmasin.'};
  if (/^(salom|assalomu alaykum|hello|привет)/.test(s)) return {kind:'hello'};
  const expenses = expenseWords.test(s), incomes = incomeWords.test(s);
  const query = /(qancha|necha|jami|hisobot|ko'rsat|bugungi|bugun|сегодня|итого)/.test(s)
    && /(xarajat|harajat|sarf|chiqim|расход)/.test(s)
    && /(qancha|necha|jami|hisobot|ko'rsat|сколько|итого|\?)/.test(s);
  if (query) return {kind:'daily'};
  if (expenses && incomes) return {kind:'invalid',reason:'Daromad va xarajatni alohida xabar qilib yuboring.'};
  if (!expenses && !incomes) return {kind:'help'};
  const amounts = findMoney(s);
  if (amounts.length !== 1) return {kind:'invalid',reason:amounts.length?'Iltimos, bitta xabarda bitta summani kiriting.':'Summani yozing. Masalan: Bugun ovqatga 50 ming so‘m sarfladim.'};
  // Prevent accidental saves of clear negative claims.
  if (/(sarflamadim|to'lamadim|ishlatmadim|xarajat qilmadim)/.test(s)) return {kind:'invalid', reason:'Bu xarajat amalga oshmagan deb tushundim. Hech narsa saqlanmadi.'};
  const amount = amounts[0];
  return {kind:incomes?'income':'expense',amount,category:inferCategory(s),comment:String(text).slice(0,140)};
}

export const formatUZS = value => new Intl.NumberFormat('uz-UZ',{maximumFractionDigits:0}).format(value) + ' so‘m';
