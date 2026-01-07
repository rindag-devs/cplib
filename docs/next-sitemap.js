/** @type {import('next-sitemap').IConfig} */
module.exports = {
  siteUrl: process.env.SITE_URL || "https://cplib.aberter0x3f.top",
  generateRobotsTxt: true,
  transform: async (config, path) => {
    let loc = path;

    if (path.endsWith(".en")) {
      loc = path.replace(/\.en$/, "");
    } else if (path.endsWith(".zh")) {
      const cleanPath = path.replace(/\.zh$/, "");
      loc = `/zh${cleanPath.startsWith("/") ? cleanPath : "/" + cleanPath}`;
    }

    return {
      loc: loc,
      changefreq: config.changefreq,
      priority: config.priority,
      lastmod: config.autoLastmod ? new Date().toISOString() : undefined,
      alternateRefs: config.alternateRefs ?? [],
    };
  },
};
