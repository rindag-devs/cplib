export default {
  docsRepositoryBase: "https://github.com/rindag-devs/cplib/tree/main/docs",
  useNextSeoProps() {
    return {
      titleTemplate: "%s - CPLib Docs",
    };
  },
  logo: (
    <div style={{ display: "flex", alignItems: "center", gap: "8px" }}>
      <strong>CPLib</strong> Docs
    </div>
  ),
  project: {
    link: "https://github.com/rindag-devs/cplib",
  },
  search: {
    component: null,
  },
  i18n: [
    { locale: "en", text: "English" },
    { locale: "zh", text: "中文" },
  ],
};