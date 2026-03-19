import type { UiConfig } from "./contracts";
import type { ConfigSectionKey, PublisherSlotKey } from "./sectionKeys";

export type LoadableSectionKey = ConfigSectionKey | `publishers.${PublisherSlotKey}`;

export type RefreshedConfigMap = Partial<
  Record<ConfigSectionKey, unknown> &
    Record<`publishers.${PublisherSlotKey}`, unknown>
>;

type ConfigLike = Partial<Omit<UiConfig, "publishers">> & {
  publishers?: Partial<UiConfig["publishers"]>;
};

export function mergeSectionValue<T extends ConfigLike>(
  current: T,
  section: LoadableSectionKey,
  value: unknown,
): T {
  if (section.startsWith("publishers.")) {
    const slot = section.replace("publishers.", "") as PublisherSlotKey;

    return {
      ...current,
      publishers: {
        ...current.publishers,
        [slot]: value,
      },
    } as T;
  }

  return {
    ...current,
    [section]: value,
  } as T;
}
